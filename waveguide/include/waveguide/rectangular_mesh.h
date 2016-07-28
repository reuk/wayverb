#pragma once

#include "boundary_adjust.h"
#include "rectangular_program.h"

#include "common/boundaries.h"
#include "common/conversions.h"
#include "common/geometric.h"
#include "common/popcount.h"

#include <set>
#include <unordered_map>
#include <vector>

namespace std {
template <typename T, typename U>
class hash<std::pair<T, U>> {
public:
    size_t operator()(const std::pair<T, U>& p) const {
        return hash<T>()(p.first) ^ hash<U>()(p.second);
    }
};
}  // namespace std

class rectangular_mesh final {
public:
    using locator = glm::ivec3;
    static constexpr auto num_ports{rectangular_program::num_ports};

    template <typename B>
    rectangular_mesh(const B& boundary, float spacing, const glm::vec3& anchor)
            : aabb(compute_adjusted_boundary(
                      boundary.get_aabb(), anchor, spacing))
            , spacing(spacing)
            , dim(dimensions(get_aabb()) / spacing)
            , nodes(compute_nodes(boundary))
            , boundary_coefficients_1(
                      compute_boundary_coefficients<1>(boundary))
            , boundary_coefficients_2(
                      compute_boundary_coefficients<2>(boundary))
            , boundary_coefficients_3(
                      compute_boundary_coefficients<3>(boundary)) {}

    template <size_t N, typename B>
    aligned::vector<std::array<cl_int, N>> compute_boundary_coefficients(
            const B& b) const {
        ConnectedFinder cf;
        aligned::vector<std::array<cl_int, N>> ret(compute_num_boundary<N>());

        for (auto i = 0u; i != get_nodes().size(); ++i) {
            const auto& node = get_nodes()[i];
            if (node.condensed.boundary_type !=
                        rectangular_program::id_reentrant &&
                popcount(node.condensed.boundary_type) == N) {
                auto count = 0;
                for (auto j = 0; j != num_ports; ++j) {
                    auto bits =
                            rectangular_program::port_index_to_boundary_type(j);
                    if (node.condensed.boundary_type & bits) {
                        auto connected = cf.look_for_connected_memoized(
                                b, i, get_nodes(), bits);
                        assert(connected.type == bits ||
                               connected.type ==
                                       rectangular_program::id_reentrant);
                        ret[node.condensed.boundary_index][count] =
                                connected.index;

                        count += 1;
                    }
                }
            }
        }

        return ret;
    }

    size_t compute_index(const locator& locator) const;
    locator compute_locator(size_t index) const;
    locator compute_locator(const glm::vec3& position) const;
    glm::vec3 compute_position(const locator& locator) const;

    const aligned::vector<rectangular_program::NodeStruct>& get_nodes() const;

    template <size_t N>
    aligned::vector<rectangular_program::BoundaryDataArray<N>>
    get_boundary_data() const {
        aligned::vector<rectangular_program::BoundaryDataArray<N>> ret;
        const auto& data_collection = get_boundary_coefficients<N>();
        ret.reserve(data_collection.size());
        for (const auto& i : data_collection) {
            ret.push_back(
                    rectangular_program::construct_boundary_data_array(i));
        }
        return ret;
    }

    void compute_neighbors(size_t index, cl_uint* output) const;
    std::array<cl_uint, num_ports> compute_neighbors(size_t index) const;

    aligned::vector<rectangular_program::CondensedNodeStruct>
    get_condensed_nodes() const;

    glm::ivec3 get_dim() const;

    template <size_t BITS>
    size_t compute_num_boundary() const {
        return std::count_if(
                get_nodes().begin(), get_nodes().end(), [](const auto& i) {
                    return i.condensed.boundary_type !=
                                   rectangular_program::id_reentrant &&
                           popcount(i.condensed.boundary_type) == BITS;
                });
    }

    size_t compute_num_reentrant() const;

    cl_int compute_boundary_type(
            const locator& loc,
            const aligned::vector<rectangular_program::NodeStruct>& ret) const;

    class ConnectedFinder {
    public:
        struct Connected {
            cl_uint index;
            cl_int type;
        };

        using MemoizeKey = std::pair<size_t, rectangular_program::BoundaryType>;

        template <typename B>
        Connected look_for_connected_memoized(
                const B& b,
                size_t node_index,
                const aligned::vector<rectangular_program::NodeStruct>& ret,
                rectangular_program::BoundaryType bt) {
            auto key   = MemoizeKey{node_index, bt};
            auto found = memoize_data.find(key);
            if (found != memoize_data.end()) {
                return found->second;
            }

            auto connected    = look_for_connected(b, node_index, ret, bt);
            memoize_data[key] = connected;
            return connected;
        }

        template <typename B>
        Connected look_for_connected(
                const B& b,
                size_t node_index,
                const aligned::vector<rectangular_program::NodeStruct>& ret,
                rectangular_program::BoundaryType bt) {
            const auto& node = ret[node_index];
            assert(node.condensed.boundary_type !=
                   rectangular_program::id_none);

            //  if this is a 1d boundary in the correct direction
            if (node.condensed.boundary_type == bt) {
                //  return the coefficient index
                return Connected{coefficient_index_for_node(b, node), bt};
            }

            //  if this is a reentrant node
            if (node.condensed.boundary_type ==
                rectangular_program::id_reentrant) {
                return Connected{coefficient_index_for_node(b, node),
                                 rectangular_program::id_reentrant};
            }

            //  if this is a 1d boundary in the wrong direction
            if (popcount(node.condensed.boundary_type) == 1) {
                return Connected{0, rectangular_program::id_none};
            }

            //  it's a higher order boundary

            //  check each adjacent node
            aligned::vector<Connected> nearby;
            for (auto i = 0; i != num_ports; ++i) {
                //  if there is (supposedly) a lower-order boundary node in this
                //  direction
                auto bits = rectangular_program::port_index_to_boundary_type(i);
                if (node.condensed.boundary_type & bits) {
                    auto adjacent_index = node.ports[i];
                    //  if the node is in the mesh
                    if (adjacent_index != rectangular_program::NO_NEIGHBOR) {
                        nearby.push_back(look_for_connected_memoized(
                                b, adjacent_index, ret, bt));
                    }
                }
            }

            assert(nearby.size() == popcount(node.condensed.boundary_type));

            auto ideal_it = std::find_if(nearby.begin(),
                                         nearby.end(),
                                         [bt](auto i) { return i.type == bt; });
            if (ideal_it != nearby.end()) {
                return *ideal_it;
            }

            auto ok_it = std::find_if(nearby.begin(), nearby.end(), [](auto i) {
                return i.type == rectangular_program::id_reentrant;
            });
            if (ok_it != nearby.end()) {
                return *ok_it;
            }

            return Connected{0, rectangular_program::id_none};
        }

    private:
        std::unordered_map<MemoizeKey, Connected> memoize_data;
    };

    float get_spacing() const;
    box get_aabb() const;

private:
    template <typename B>
    aligned::vector<rectangular_program::NodeStruct> compute_nodes(
            const B& boundary) const {
        const auto dim   = get_dim();
        auto total_nodes = dim.x * dim.y * dim.z;
        auto ret         = aligned::vector<rectangular_program::NodeStruct>(
                total_nodes, rectangular_program::NodeStruct{});
        set_node_positions(ret);
        set_node_inside(boundary, ret);
        set_node_boundary_type(ret);
        set_node_boundary_index(ret);

        return ret;
    }

    static cl_uint coefficient_index_for_node(
            const Boundary& b, const rectangular_program::NodeStruct& node);
    static cl_uint coefficient_index_for_node(
            const MeshBoundary& b, const rectangular_program::NodeStruct& node);

    void set_node_positions(
            aligned::vector<rectangular_program::NodeStruct>& ret) const;
    void set_node_inside(
            const Boundary& boundary,
            aligned::vector<rectangular_program::NodeStruct>& ret) const;
    void set_node_boundary_type(
            aligned::vector<rectangular_program::NodeStruct>& ret) const;

    template <int I>
    void set_node_boundary_index(
            aligned::vector<rectangular_program::NodeStruct>& ret) const {
        auto num_boundary = 0;
        for (auto& i : ret) {
            if (i.condensed.boundary_type !=
                        rectangular_program::id_reentrant &&
                popcount(i.condensed.boundary_type) == I) {
                i.condensed.boundary_index = num_boundary++;
            }
        }
    }

    void set_node_boundary_index(
            aligned::vector<rectangular_program::NodeStruct>& ret) const;

    template <size_t N>
    const aligned::vector<std::array<cl_int, N>>& get_boundary_coefficients()
            const;

    box aabb;
    float spacing;

    glm::ivec3 dim;

    aligned::vector<rectangular_program::NodeStruct> nodes;
    aligned::vector<std::array<cl_int, 1>> boundary_coefficients_1;
    aligned::vector<std::array<cl_int, 2>> boundary_coefficients_2;
    aligned::vector<std::array<cl_int, 3>> boundary_coefficients_3;

    friend bool operator==(const rectangular_mesh& a,
                           const rectangular_mesh& b);
};

template <>
inline const aligned::vector<std::array<cl_int, 1>>&
rectangular_mesh::get_boundary_coefficients() const {
    return boundary_coefficients_1;
}
template <>
inline const aligned::vector<std::array<cl_int, 2>>&
rectangular_mesh::get_boundary_coefficients() const {
    return boundary_coefficients_2;
}
template <>
inline const aligned::vector<std::array<cl_int, 3>>&
rectangular_mesh::get_boundary_coefficients() const {
    return boundary_coefficients_3;
}

bool operator==(const rectangular_mesh& a, const rectangular_mesh& b);
