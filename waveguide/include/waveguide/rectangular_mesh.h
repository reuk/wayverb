#pragma once

#include "base_mesh.h"
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

class RectangularMesh : public BaseMesh<rectangular_program, glm::ivec3> {
public:
    template <typename B>
    RectangularMesh(const B& boundary, float spacing, const glm::vec3& anchor)
            : BaseMesh(spacing,
                       compute_adjusted_boundary(
                               boundary.get_aabb(), anchor, spacing))
            , dim(get_aabb().dimensions() / spacing)
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
            if (node.boundary_type != rectangular_program::id_reentrant &&
                popcount(node.boundary_type) == N) {
                auto count = 0;
                for (auto j = 0; j != PORTS; ++j) {
                    auto bits =
                            rectangular_program::port_index_to_boundary_type(j);
                    if (node.boundary_type & bits) {
                        auto connected = cf.look_for_connected_memoized(
                                b, i, get_nodes(), bits);
                        assert(connected.type == bits ||
                               connected.type ==
                                       rectangular_program::id_reentrant);
                        ret[node.boundary_index][count] = connected.index;

                        count += 1;
                    }
                }
            }
        }

        return ret;
    }

    using CondensedNode = rectangular_program::CondensedNodeStruct;

    size_type compute_index(const Locator& locator) const override;
    Locator compute_locator(const size_type index) const override;
    Locator compute_locator(const glm::vec3& position) const override;
    glm::vec3 compute_position(const Locator& locator) const override;

    const Collection& get_nodes() const override;

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

    void compute_neighbors(size_type index, cl_uint* output) const override;

    aligned::vector<CondensedNode> get_condensed_nodes() const;

    glm::ivec3 get_dim() const;

    template <size_t BITS>
    size_type compute_num_boundary() const {
        return std::count_if(
                get_nodes().begin(), get_nodes().end(), [](const auto& i) {
                    return i.boundary_type !=
                                   rectangular_program::id_reentrant &&
                           popcount(i.boundary_type) == BITS;
                });
    }

    size_type compute_num_reentrant() const;

    cl_int compute_boundary_type(const Locator& loc,
                                 const aligned::vector<Node>& ret) const;

    class ConnectedFinder {
    public:
        struct Connected {
            cl_uint index;
            cl_int type;
        };

        using MemoizeKey = std::pair<aligned::vector<Node>::size_type,
                                     rectangular_program::BoundaryType>;

        template <typename B>
        Connected look_for_connected_memoized(
                const B& b,
                aligned::vector<Node>::size_type node_index,
                const aligned::vector<Node>& ret,
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
                aligned::vector<Node>::size_type node_index,
                const aligned::vector<Node>& ret,
                rectangular_program::BoundaryType bt) {
            const auto& node = ret[node_index];
            assert(node.boundary_type != rectangular_program::id_none);

            //  if this is a 1d boundary in the correct direction
            if (node.boundary_type == bt) {
                //  return the coefficient index
                return Connected{coefficient_index_for_node(b, node), bt};
            }

            //  if this is a reentrant node
            if (node.boundary_type == rectangular_program::id_reentrant) {
                return Connected{coefficient_index_for_node(b, node),
                                 rectangular_program::id_reentrant};
            }

            //  if this is a 1d boundary in the wrong direction
            if (popcount(node.boundary_type) == 1) {
                return Connected{0, rectangular_program::id_none};
            }

            //  it's a higher order boundary

            //  check each adjacent node
            aligned::vector<Connected> nearby;
            for (auto i = 0; i != PORTS; ++i) {
                //  if there is (supposedly) a lower-order boundary node in this
                //  direction
                auto bits = rectangular_program::port_index_to_boundary_type(i);
                if (node.boundary_type & bits) {
                    auto adjacent_index = node.ports[i];
                    //  if the node is in the mesh
                    if (adjacent_index != rectangular_program::NO_NEIGHBOR) {
                        nearby.push_back(look_for_connected_memoized(
                                b, adjacent_index, ret, bt));
                    }
                }
            }

            assert(nearby.size() == popcount(node.boundary_type));

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

private:
    template <typename B>
    Collection compute_nodes(const B& boundary) const {
        const auto dim   = get_dim();
        auto total_nodes = dim.x * dim.y * dim.z;
        //        auto bytes = total_nodes *
        //        sizeof(RectangularMesh::CondensedNode);
        // LOG(INFO) << (bytes >> 20) << " MB required for node metadata
        // storage!";

        //  we will return this eventually
        auto ret = aligned::vector<Node>(total_nodes, Node{});

        set_node_positions(ret);
        set_node_inside(boundary, ret);
        set_node_boundary_type(ret);
        set_node_boundary_index(ret);

        return ret;
    }

    static cl_uint coefficient_index_for_node(const Boundary& b,
                                              const Node& node);
    static cl_uint coefficient_index_for_node(const MeshBoundary& b,
                                              const Node& node);

    void set_node_positions(aligned::vector<Node>& ret) const;
    void set_node_inside(const Boundary& boundary,
                         aligned::vector<Node>& ret) const;
    void set_node_boundary_type(aligned::vector<Node>& ret) const;

    template <int I>
    void set_node_boundary_index(aligned::vector<Node>& ret) const {
        auto num_boundary = 0;
        for (auto& i : ret) {
            if (i.boundary_type != rectangular_program::id_reentrant &&
                popcount(i.boundary_type) == I) {
                i.boundary_index = num_boundary++;
            }
        }
    }

    void set_node_boundary_index(aligned::vector<Node>& ret) const;

    template <size_t N>
    const aligned::vector<std::array<cl_int, N>>& get_boundary_coefficients()
            const;

    glm::ivec3 dim;

    Collection nodes;
    aligned::vector<std::array<cl_int, 1>> boundary_coefficients_1;
    aligned::vector<std::array<cl_int, 2>> boundary_coefficients_2;
    aligned::vector<std::array<cl_int, 3>> boundary_coefficients_3;

    friend bool operator==(const RectangularMesh& a, const RectangularMesh& b);
};

template <>
inline const aligned::vector<std::array<cl_int, 1>>&
RectangularMesh::get_boundary_coefficients() const {
    return boundary_coefficients_1;
}
template <>
inline const aligned::vector<std::array<cl_int, 2>>&
RectangularMesh::get_boundary_coefficients() const {
    return boundary_coefficients_2;
}
template <>
inline const aligned::vector<std::array<cl_int, 3>>&
RectangularMesh::get_boundary_coefficients() const {
    return boundary_coefficients_3;
}

bool operator==(const RectangularMesh& a, const RectangularMesh& b);