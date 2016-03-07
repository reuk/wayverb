#pragma once

#include "vec.h"
#include "boundaries.h"
#include "base_mesh.h"
#include "rectangular_program.h"
#include "geometric.h"
#include "conversions.h"

#include <vector>
#include <set>

class RectangularMesh : public BaseMesh<RectangularProgram, Vec3i> {
public:
    RectangularMesh(const MeshBoundary& boundary,
                    float spacing,
                    const Vec3f& anchor);
    RectangularMesh(const Boundary& boundary,
                    float spacing,
                    const Vec3f& anchor);

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
        return std::accumulate(
            get_nodes().begin(),
            get_nodes().end(),
            0u,
            [](auto i, const auto& j) {
                return i +
                       ((j.boundary_type != RectangularProgram::id_reentrant &&
                         popcount(j.boundary_type) == BITS)
                            ? 1
                            : 0);
            });
    }

private:
    Collection compute_nodes(const Boundary& boundary) const;

    //  for a normal boundary, in the 1d case, we just take the first surface
    //  that the boundary owns and apply it to all 1d boundary nodes
    std::vector<RectangularProgram::BoundaryDataArray1> compute_boundary_data_1(
        const Boundary& boundary) const;

    //  for a mesh boundary, in the 1d case, we want to find the closest
    //  triangle to each boundary node, and then apply that triangle's surface
    //  to the node
    std::vector<RectangularProgram::BoundaryDataArray1> compute_boundary_data_1(
        const MeshBoundary& boundary) const;

    template <int I>
    std::vector<std::pair<RectangularProgram::BoundaryType, cl_uint>>
    compute_coefficient_indices(const Node& node) const {
        if (popcount(node.boundary_type) != I)
            throw std::runtime_error("bad bitcount in node boundary type");

        std::vector<std::pair<RectangularProgram::BoundaryType, cl_uint>> ret;
        for (auto i = 0; i != PORTS; ++i) {
            auto bits = RectangularProgram::port_index_to_boundary_type(i);
            if (all_flags_set(node.boundary_type, std::make_tuple(bits))) {
                //  get the adjacent boundary node in this direction
                auto to_merge = compute_coefficient_indices<I - 1>(
                    get_nodes()[node.ports[i]]);
                //  insert into return set
                std::vector<std::pair<RectangularProgram::BoundaryType,
                                      cl_uint>> merged;
                std::set_union(to_merge.begin(),
                               to_merge.end(),
                               ret.begin(),
                               ret.end(),
                               std::back_inserter(merged));
                ret = merged;
            }
        }

        if (ret.size() != I)
            throw std::runtime_error("found wrong number of unique boundaries");

        std::sort(
            ret.begin(),
            ret.end(),
            [](const auto& i, const auto& j) { return i.first < j.first; });
        return ret;
    }

    //  in the 2d and 3d cases, we want to look at the adjacent boundary nodes
    //  and just use the same boundaries
    //
    //  the 1d case is a bit different, so there's a specialization for it
    template <int I>
    std::vector<RectangularProgram::BoundaryDataArray<I>>
    compute_boundary_data() const {
        std::vector<RectangularProgram::BoundaryDataArray<I>> ret(
            compute_num_boundary<I>());
        //  for each node
        for (const auto& node : get_nodes()) {
            //  if node is the correct type of boundary node
            if (popcount(node.boundary_type) == I) {
                //  find adjacent boundary nodes
                auto indices = compute_coefficient_indices<I>(node);
                for (auto i = 0; i != I; ++i) {
                    ret[node.boundary_index].array[i].coefficient_index =
                        indices[i].second;
                }
            }
        }
        return ret;
    }

    void set_node_positions(std::vector<Node>& ret) const;
    void set_node_inside(const Boundary& boundary,
                         std::vector<Node>& ret) const;
    void set_node_boundary_type(std::vector<Node>& ret) const;

    template <int I>
    void set_node_boundary_index(std::vector<Node>& ret) const {
        auto num_boundary = 0;
        for (auto& i : ret) {
            if (popcount(i.boundary_type) == I) {
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

template <>
inline std::vector<std::pair<RectangularProgram::BoundaryType, cl_uint>>
RectangularMesh::compute_coefficient_indices<1>(const Node& node) const {
    assert(popcount(node.boundary_type) == 1);
    return {std::pair<RectangularProgram::BoundaryType, cl_uint>(
        static_cast<RectangularProgram::BoundaryType>(node.boundary_type),
        get_boundary_data<1>()[node.boundary_index]
            .array[0]
            .coefficient_index)};
}
