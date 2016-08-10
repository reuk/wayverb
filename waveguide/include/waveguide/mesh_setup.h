#pragma once

#include "common/aligned/vector.h"
#include "common/cl_include.h"
#include "common/program_wrapper.h"
#include "common/stl_wrappers.h"
#include "common/voxel_collection.h"

#include "glm/fwd.hpp"

class copyable_scene_data;

namespace waveguide {

class mesh_boundary;

namespace mesh_setup {

typedef enum : cl_int {
    id_none = 0,
    id_inside = 1 << 0,
    id_nx = 1 << 1,
    id_px = 1 << 2,
    id_ny = 1 << 3,
    id_py = 1 << 4,
    id_nz = 1 << 5,
    id_pz = 1 << 6,
    id_reentrant = 1 << 7,
} boundary_type;

constexpr boundary_type port_index_to_boundary_type(unsigned int i) {
    return static_cast<boundary_type>(1 << (i + 1));
}

constexpr cl_uint no_neighbor{~cl_uint{0}};

struct alignas(1 << 4) node final {
    static constexpr size_t num_ports{6};

    cl_uint ports[num_ports]{};  /// the indices of adjacent ports
    cl_float3 position{};        /// spatial position
    cl_char inside{};            /// is the node an air node?
    cl_int boundary_type{};      /// describes the boundary type
    cl_uint boundary_index{};    /// an index into a boundary descriptor array
};

inline bool operator==(const node& a, const node& b) {
    return proc::equal(a.ports, std::begin(b.ports)) &&
           std::tie(a.position, a.inside, a.boundary_type, a.boundary_index) ==
                   std::tie(b.position,
                            b.inside,
                            b.boundary_type,
                            b.boundary_index);
}

inline bool operator!=(const node& a, const node& b) { return !(a == b); }

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

class program final {
public:
    program(const cl::Context& context, const cl::Device& device);

    auto get_node_position_and_neighbors_kernel() const {
        return program_wrapper.get_kernel<cl::Buffer,  /// nodes
                                          cl_int3,     /// dim
                                          cl_float3,   /// min_corner
                                          cl_float     /// spacing
                                          >("set_node_position_and_neighbors");
    }

private:
    static const std::string source;

    program_wrapper program_wrapper;
};

aligned::vector<node> do_mesh_setup(const cl::Context& context,
                                    const cl::Device& device,
                                    const copyable_scene_data& scene_data,
                                    const voxel_collection<3>& octree,
                                    float spacing,
                                    const glm::vec3& anchor);

}  // namespace mesh_setup
}  // namespace waveguide
