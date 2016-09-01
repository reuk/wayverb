#pragma once

#include "waveguide/mesh_descriptor.h"

#include "common/cl/voxel_structs.h"
#include "common/program_wrapper.h"

namespace waveguide {
class boundary_coefficient_program final {
public:
    boundary_coefficient_program(const compute_context& cc);

    auto get_boundary_coefficient_finder_1d_kernel() const {
        return wrapper.get_kernel<cl::Buffer,       /// nodes
                                  mesh_descriptor,  /// descriptor
                                  cl::Buffer,       /// 1d boundary index
                                  cl::Buffer,       /// voxel_index
                                  aabb,             /// global_aabb
                                  cl_uint,          /// side
                                  cl::Buffer,       /// triangles
                                  cl_uint,          /// num_triangles
                                  cl::Buffer        /// vertices
                                  >("boundary_coefficient_finder_1d");
    }

    auto get_boundary_coefficient_finder_2d_kernel() const {
        return wrapper.get_kernel<cl::Buffer,       /// nodes
                                  mesh_descriptor,  /// descriptor
                                  cl::Buffer,       /// 2d boundary index
                                  cl::Buffer        /// 1d boundary index
                                  >("boundary_coefficient_finder_2d");
    }

    auto get_boundary_coefficient_finder_3d_kernel() const {
        return wrapper.get_kernel<cl::Buffer,       /// nodes
                                  mesh_descriptor,  /// descriptor
                                  cl::Buffer,       /// 3d boundary index
                                  cl::Buffer        /// 1d boundary index
                                  >("boundary_coefficient_finder_3d");
    }

private:
    program_wrapper wrapper;
};

}  // namespace waveguide
