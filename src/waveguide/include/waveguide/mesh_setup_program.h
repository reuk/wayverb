#pragma once

#include "waveguide/mesh_descriptor.h"

#include "core/cl/voxel_structs.h"
#include "core/program_wrapper.h"

namespace waveguide {

class setup_program final {
public:
    setup_program(const compute_context& cc);

    auto get_node_inside_kernel() const {
        return program_wrapper.get_kernel<cl::Buffer,       /// nodes
                                          mesh_descriptor,  /// descriptor
                                          cl::Buffer,       /// voxel_index
                                          aabb,             /// global_aabb
                                          cl_uint,          /// side
                                          cl::Buffer,       /// triangles
                                          cl::Buffer        /// vertices
                                          >("set_node_inside");
    }

    auto get_node_boundary_kernel() const {
        return program_wrapper.get_kernel<cl::Buffer,      /// nodes
                                          mesh_descriptor  /// descriptor
                                          >("set_node_boundary_type");
    }

private:
    program_wrapper program_wrapper;
};

}  // namespace waveguide
