#pragma once

#include "vec.h"
#include "boundaries.h"
#include "base_mesh.h"
#include "rectangular_program.h"
#include "geometric.h"
#include "conversions.h"
#include "logger.h"
#include "boundary_adjust.h"

#include <vector>
#include <set>

template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, const std::pair<T, U>& p) {
    Bracketer bracketer(os);
    return to_stream(os, p.first, "  ", p.second, "  ");
}

template <typename T, unsigned long U>
std::ostream& operator<<(std::ostream& os, const std::array<T, U>& arr) {
    Bracketer bracketer(os);
    for (const auto& i : arr)
        to_stream(os, i, "  ");
    return os;
}

class RectangularMesh : public BaseMesh<RectangularProgram, Vec3i> {
public:
    template <typename B>
    RectangularMesh(const B& boundary, float spacing, const Vec3f& anchor)
            : BaseMesh(spacing,
                       compute_adjusted_boundary(
                           boundary.get_aabb(), anchor, spacing))
            , dim(get_aabb().get_dimensions() / spacing)
            , nodes(compute_nodes(boundary))
            , boundary_data_1(compute_boundary_data_1(boundary))
            , boundary_data_2(compute_boundary_data<2>())
            , boundary_data_3(compute_boundary_data<3>())
    {
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

private:
    void log_node_stats(const std::vector<Node>& ret) const;

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

        compute_reentrant_coefficient_indices(ret, boundary);

        return ret;
    }

    static cl_int coefficient_index_for_node(const Boundary& b,
                                             const Node& node);
    static cl_int coefficient_index_for_node(const MeshBoundary& b,
                                             const Node& node);

    template <typename B>
    std::vector<RectangularProgram::BoundaryDataArray1> compute_boundary_data_1(
        const B& boundary) const {
        std::vector<RectangularProgram::BoundaryDataArray1> ret(
            compute_num_boundary<1>());
        for (const auto& node : get_nodes()) {
            if (popcount(node.boundary_type) == 1 &&
                node.boundary_type != RectangularProgram::id_reentrant) {
                ret[node.boundary_index].array[0].coefficient_index =
                    coefficient_index_for_node(boundary, node);
            }
        }
        return ret;
    }

    //  for a given node
    //  if it's a 1d boundary node
    //      find the closest material to that node
    //  if it's a 2d boundary node
    //      adjacent nodes will be 1d nodes or reentrant nodes

    std::vector<std::pair<cl_int, cl_uint>> compute_coefficient_indices(const Node& node)const;

    template <int I>
    std::vector<std::pair<cl_int, cl_uint>>
    compute_coefficient_indices(const Node& node) const {
        if (popcount(node.boundary_type) != I)
            throw std::runtime_error("bad bitcount in node boundary type");
        std::vector<std::pair<cl_int, cl_uint>> ret;

        std::vector<cl_int> bt;

        //  for each adjacent port
        for (auto i = 0; i != PORTS; ++i) {
            auto bits = RectangularProgram::port_index_to_boundary_type(i);
            //  check if there's a boundary node in that direction
            if (all_flags_set(node.boundary_type, std::make_tuple(bits))) {

                //  get adjacent node
                const auto& adjacent = get_nodes()[node.ports[i]];

                bt.push_back(adjacent.boundary_type);

                //  find node classification
                std::vector<std::pair<cl_int, cl_uint>> to_merge;
                if (adjacent.boundary_type ==
                    RectangularProgram::id_reentrant) {
                    to_merge.push_back(std::make_pair(bits | RectangularProgram::id_reentrant, adjacent.boundary_index));
                } else {
                    to_merge = compute_coefficient_indices(adjacent);
                }

                //  insert into return set
                std::vector<std::pair<cl_int, cl_uint>> merged;
                std::set_union(to_merge.begin(),
                               to_merge.end(),
                               ret.begin(),
                               ret.end(),
                               std::back_inserter(merged));
                ret = std::move(merged);
            }
        }

        if (bt.size() != I)
            throw std::runtime_error("wtf");

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
                assert(indices.size() == I);
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
            if (i.boundary_type != RectangularProgram::id_reentrant &&
                popcount(i.boundary_type) == I) {
                i.boundary_index = num_boundary++;
            }
        }
    }

    void set_node_boundary_index(std::vector<Node>& ret) const;

    template <typename B>
    void compute_reentrant_coefficient_indices(std::vector<Node>& ret,
                                               const B& boundary) const {
        for (auto& node : ret) {
            if (node.boundary_type == RectangularProgram::id_reentrant) {
                node.boundary_index =
                    coefficient_index_for_node(boundary, node);
            }
        }
    }

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
inline std::vector<std::pair<cl_int, cl_uint>>
RectangularMesh::compute_coefficient_indices<1>(const Node& node) const {
    assert(popcount(node.boundary_type) == 1 &&
           node.boundary_type !=
               RectangularProgram::BoundaryType::id_reentrant);
    return {{std::make_pair(
        node.boundary_type,
        get_boundary_data<1>()[node.boundary_index]
            .array[0]
            .coefficient_index)}};
}
