#pragma once

#include "waveguide/cl/structs.h"
#include "waveguide/cl/utils.h"
#include "waveguide/mesh/descriptor.h"

#include "common/cl/cl_representation.h"
#include "common/program_wrapper.h"
#include "common/spatial_division/scene_buffers.h"

template <size_t n>
struct alignas(1 << 2) boundary_index_array final {
    cl_uint array[n];
};

using boundary_index_array_1 = boundary_index_array<1>;
using boundary_index_array_2 = boundary_index_array<2>;
using boundary_index_array_3 = boundary_index_array<3>;

template <>
struct cl_representation<boundary_index_array_1> final {
    static constexpr auto value{R"(
typedef struct { uint array[1]; } boundary_index_array_1;
)"};
};

template <>
struct cl_representation<boundary_index_array_2> final {
    static constexpr auto value{R"(
typedef struct { uint array[2]; } boundary_index_array_2;
)"};
};

template <>
struct cl_representation<boundary_index_array_3> final {
    static constexpr auto value{R"(
typedef struct { uint array[3]; } boundary_index_array_3;
)"};
};

namespace waveguide {
namespace mesh {

class boundary_coefficient_program final {
public:
    boundary_coefficient_program(const cl::Context& context,
                                 const cl::Device& device);

    auto get_boundary_coefficient_finder_1d_kernel() const {
        return wrapper.get_kernel<cl::Buffer,  /// nodes
                                  cl::Buffer,  /// 1d boundary index
                                  cl::Buffer,  /// voxel_index
                                  aabb,        /// global_aabb
                                  cl_uint,     /// side
                                  cl::Buffer,  /// triangles
                                  cl::Buffer   /// vertices
                                  >("boundary_coefficient_finder_1d");
    }

    auto get_boundary_coefficient_finder_2d_kernel() const {
        return wrapper.get_kernel<cl::Buffer,  /// nodes
                                  cl_int3,     /// dim
                                  cl::Buffer,  /// 2d boundary index
                                  cl::Buffer   /// 1d boundary index
                                  >("boundary_coefficient_finder_2d");
    }

    auto get_boundary_coefficient_finder_3d_kernel() const {
        return wrapper.get_kernel<cl::Buffer,  /// nodes
                                  cl_int3,     /// dim
                                  cl::Buffer,  /// 3d boundary index
                                  cl::Buffer   /// 1d boundary index
                                  >("boundary_coefficient_finder_3d");
    }

private:
    program_wrapper wrapper;
};

//  or maybe keep the buffers on the gpu?
using boundary_index_data = std::tuple<aligned::vector<boundary_index_array_1>,
                                       aligned::vector<boundary_index_array_2>,
                                       aligned::vector<boundary_index_array_3>>;

boundary_index_data compute_boundary_index_data(const cl::Device& device,
                                                const scene_buffers& buffers,
                                                aligned::vector<node>& nodes,
                                                const descriptor& desc);

}  // namespace mesh
}  // namespace waveguide
