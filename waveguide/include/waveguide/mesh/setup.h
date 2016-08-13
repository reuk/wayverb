#pragma once

#include "waveguide/program.h"

#include "common/aligned/vector.h"
#include "common/cl_include.h"
#include "common/map_to_vector.h"
#include "common/program_wrapper.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxel_collection.h"
#include "common/stl_wrappers.h"
#include "waveguide/cl/utils.h"
#include "waveguide/mesh/descriptor.h"

#include "glm/fwd.hpp"

class copyable_scene_data;

namespace waveguide {

class mesh_boundary;

namespace mesh {

//----------------------------------------------------------------------------//

/// This program will set up a rectangular mesh, given a bunch of room geometry
/// and the number of nodes.
///
///     first, find each node's position
///         easy
///     then, find whether the node is inside or outside
///         less easy
///         voxels -> need some kind of 'counter-traversal'
///         modify existing voxel_traversal I guess
///     calculate what type of boundary the node represents
///     find the boundary node index for each node

class setup_program final {
public:
    setup_program(const cl::Context& context, const cl::Device& device);

    auto get_node_position_and_neighbors_kernel() const {
        return program_wrapper.get_kernel<cl::Buffer,  /// nodes
                                          cl_int3,     /// dim
                                          cl_float3,   /// min_corner
                                          cl_float     /// spacing
                                          >("set_node_position_and_neighbors");
    }

    auto get_node_inside_kernel() const {
        return program_wrapper.get_kernel<cl::Buffer,  /// nodes
                                          cl::Buffer,  /// voxel_index
                                          aabb,        /// global_aabb
                                          cl_ulong,    /// side
                                          cl::Buffer,  /// triangles
                                          cl::Buffer   /// vertices
                                          >("set_node_inside");
    }

private:
    static const std::string source;

    program_wrapper program_wrapper;
};

//----------------------------------------------------------------------------//

condensed_node get_condensed(const node& n);
aligned::vector<condensed_node> get_condensed(const aligned::vector<node>& n);

constexpr bool is_inside(const condensed_node& c) {
    return c.boundary_type && id_inside;
}

//----------------------------------------------------------------------------//

class vectors final {
public:
    vectors(const aligned::vector<node>& nodes,
            const aligned::vector<canonical_coefficients>& coefficients);

    template <size_t n>
    const aligned::vector<std::array<cl_uint, n>>& get_boundary_coefficients()
            const;

    const aligned::vector<condensed_node>& get_condensed_nodes() const;
    const aligned::vector<canonical_coefficients>& get_coefficients() const;

private:
    aligned::vector<condensed_node> condensed_nodes;
    aligned::vector<canonical_coefficients> coefficients;
    aligned::vector<std::array<cl_uint, 1>> boundary_coefficients_1;
    aligned::vector<std::array<cl_uint, 2>> boundary_coefficients_2;
    aligned::vector<std::array<cl_uint, 3>> boundary_coefficients_3;
};

template <>
inline const aligned::vector<std::array<cl_uint, 1>>&
vectors::get_boundary_coefficients() const {
    return boundary_coefficients_1;
}
template <>
inline const aligned::vector<std::array<cl_uint, 2>>&
vectors::get_boundary_coefficients() const {
    return boundary_coefficients_2;
}
template <>
inline const aligned::vector<std::array<cl_uint, 3>>&
vectors::get_boundary_coefficients() const {
    return boundary_coefficients_3;
}

//----------------------------------------------------------------------------//

template <size_t N>
static boundary_data_array<N> construct_boundary_data_array(
        const std::array<cl_uint, N>& arr) {
    boundary_data_array<N> ret{};
    for (auto i = 0u; i != N; ++i) {
        ret.array[i].coefficient_index = arr[i];
    }
    return ret;
}

template <size_t n>
inline aligned::vector<boundary_data_array<n>> get_boundary_data(
        const vectors& d) {
    return map_to_vector(d.get_boundary_coefficients<n>(), [](const auto& i) {
        return construct_boundary_data_array(i);
    });
}

}  // namespace mesh
}  // namespace waveguide
