#pragma once

#include "vec.h"
#include "boundaries.h"
#include "base_mesh.h"
#include "rectangular_program.h"
#include "geometric.h"
#include "conversions.h"
#include "logger.h"
#include "boundary_adjust.h"
#include "ostream_overloads.h"

#include <vector>
#include <set>

class RectangularMesh : public BaseMesh<RectangularProgram, Vec3i> {
public:
    template <typename B>
    RectangularMesh(const B& boundary, float spacing, const Vec3f& anchor)
            : BaseMesh(spacing,
                       compute_adjusted_boundary(
                           boundary.get_aabb(), anchor, spacing))
            , dim(get_aabb().get_dimensions() / spacing)
            , nodes(compute_nodes(boundary))
            , boundary_data_1(compute_boundary_data<1>(boundary))
            , boundary_data_2(compute_boundary_data<2>(boundary))
            , boundary_data_3(compute_boundary_data<3>(boundary)) {
    }

    template <int I, typename B>
    std::vector<RectangularProgram::BoundaryDataArray<I>> compute_boundary_data(
        const B& b) const {
        std::vector<RectangularProgram::BoundaryDataArray<I>> ret(
            compute_num_boundary<I>());

        for (const auto& i : get_nodes()) {
            if (i.boundary_type != RectangularProgram::id_reentrant &&
                popcount(i.boundary_type) == I) {
                auto count = 0;
                for (auto j = 0; j != PORTS; ++j) {
                    auto bits =
                        RectangularProgram::port_index_to_boundary_type(j);
                    if (i.boundary_type & bits) {
                        auto connected =
                            look_for_connected(b, i, get_nodes(), bits);
                        assert(connected.type == bits ||
                               connected.type ==
                                   RectangularProgram::id_reentrant);
                        ret[i.boundary_index].array[count++].coefficient_index =
                            connected.index;
                    }
                }
            }
        }

        return ret;
    }

    using CondensedNode = RectangularProgram::CondensedNodeStruct;

    size_type compute_index(const Locator& locator) const override;
    Locator compute_locator(const size_type index) const override;
    Locator compute_locator(const Vec3f& position) const override;
    Vec3f compute_position(const Locator& locator) const override;

    const Collection& get_nodes() const override;

    template <int I>
    const std::vector<RectangularProgram::BoundaryDataArray<I>>&
    get_boundary_data() const;

    void compute_neighbors(size_type index, cl_uint* output) const override;

    std::vector<CondensedNode> get_condensed_nodes() const;

    Vec3i get_dim() const;

    template <int BITS>
    size_type compute_num_boundary() const {
        return std::count_if(get_nodes().begin(),
                             get_nodes().end(),
                             [](const auto& i) {
                                 return i.boundary_type !=
                                            RectangularProgram::id_reentrant &&
                                        popcount(i.boundary_type) == BITS;
                             });
    }

    size_type compute_num_reentrant() const;

    cl_int compute_boundary_type(const Locator& loc,
                                 const std::vector<Node>& ret) const;

    struct Connected {
        cl_uint index;
        cl_int type;
    };

    template <typename B>
    Connected look_for_connected(const B& b,
                                 const Node& node,
                                 const std::vector<Node>& ret,
                                 RectangularProgram::BoundaryType bt) const {
        assert(node.boundary_type != RectangularProgram::id_none);

        //  if this is a 1d boundary in the correct direction
        if (node.boundary_type == bt) {
            //  return the coefficient index
            return Connected{coefficient_index_for_node(b, node), bt};
        }

        //  if this is a reentrant node
        if (node.boundary_type == RectangularProgram::id_reentrant) {
            return Connected{coefficient_index_for_node(b, node),
                             RectangularProgram::id_reentrant};
        }

        //  if this is a 1d boundary in the wrong direction
        if (popcount(node.boundary_type) == 1) {
            return Connected{0, RectangularProgram::id_none};
        }

        //  it's a higher order boundary

        //  check each adjacent node
        std::vector<Connected> nearby;
        for (auto i = 0; i != PORTS; ++i) {
            //  if there is (supposedly) a lower-order boundary node in this
            //  direction
            auto bits = RectangularProgram::port_index_to_boundary_type(i);
            if (node.boundary_type & bits) {
                auto adjacent_index = node.ports[i];
                //  if the node is in the mesh
                if (adjacent_index != RectangularProgram::NO_NEIGHBOR) {
                    const auto& adjacent = ret[adjacent_index];
                    nearby.push_back(look_for_connected(b, adjacent, ret, bt));
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

        auto ok_it = std::find_if(
            nearby.begin(),
            nearby.end(),
            [](auto i) { return i.type == RectangularProgram::id_reentrant; });
        if (ok_it != nearby.end()) {
            return *ok_it;
        }

        return Connected{0, RectangularProgram::id_none};
    }

private:
    template <typename B>
    Collection compute_nodes(const B& boundary) const {
        auto total_nodes = get_dim().product();
        auto bytes = total_nodes * sizeof(RectangularMesh::CondensedNode);
        ::Logger::log_err(bytes >> 20,
                          " MB required for node metadata storage!");

        //  we will return this eventually
        auto ret = std::vector<Node>(total_nodes, Node{});

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

    void set_node_positions(std::vector<Node>& ret) const;
    void set_node_inside(const Boundary& boundary,
                         std::vector<Node>& ret) const;
    void set_node_boundary_type(std::vector<Node>& ret) const;

    template <int I>
    void set_node_boundary_index(std::vector<Node>& ret) const {
        auto num_boundary = 0;
        for (auto& i : ret) {
            if (i.boundary_type != RectangularProgram::id_reentrant &&
                popcount(i.boundary_type) == I) {
                i.boundary_index = num_boundary++;
            }
        }
    }

    void set_node_boundary_index(std::vector<Node>& ret) const;

    Vec3i dim;

    Collection nodes;
    std::vector<RectangularProgram::BoundaryDataArray1> boundary_data_1;
    std::vector<RectangularProgram::BoundaryDataArray2> boundary_data_2;
    std::vector<RectangularProgram::BoundaryDataArray3> boundary_data_3;
};

template <>
inline const std::vector<RectangularProgram::BoundaryDataArray<1>>&
RectangularMesh::get_boundary_data<1>() const {
    return boundary_data_1;
}

template <>
inline const std::vector<RectangularProgram::BoundaryDataArray<2>>&
RectangularMesh::get_boundary_data<2>() const {
    return boundary_data_2;
}

template <>
inline const std::vector<RectangularProgram::BoundaryDataArray<3>>&
RectangularMesh::get_boundary_data<3>() const {
    return boundary_data_3;
}
