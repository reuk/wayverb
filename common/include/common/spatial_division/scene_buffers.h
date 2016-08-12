#pragma once

#include "common/cl_traits.h"

class voxelised_scene_data;

struct alignas(1 << 4) aabb final {
    cl_float3 c0;
    cl_float3 c1;
};

constexpr auto to_tuple(const aabb& x) { return std::tie(x.c0, x.c1); }

constexpr bool operator==(const aabb& a, const aabb& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const aabb& a, const aabb& b) { return !(a == b); }

//----------------------------------------------------------------------------//

/// Provides a simple utility for loading voxelised scene data to the gpu in
/// one go.
class scene_buffers final {
public:
    scene_buffers(const cl::Context&, const voxelised_scene_data& scene_data);

    const cl::Buffer& get_voxel_index_buffer() const;
    aabb get_global_aabb() const;
    cl_ulong get_side() const;

    const cl::Buffer& get_triangles_buffer() const;
    const cl::Buffer& get_vertices_buffer() const;
    const cl::Buffer& get_surfaces_buffer() const;

private:
    const cl::Buffer voxel_index_buffer;
    const aabb global_aabb;
    const cl_ulong side;

    const cl::Buffer triangles_buffer;
    const cl::Buffer vertices_buffer;
    const cl::Buffer surfaces_buffer;
};
