#pragma once

#include "common/cl/voxel_structs.h"
#include "common/spatial_division/voxelised_scene_data.h"

/// Provides a simple utility for loading voxelised scene data to the gpu in
/// one go.
class scene_buffers final {
public:
    scene_buffers(const cl::Context&,
                  const voxelised_scene_data<cl_float3, surface>& scene_data);

    cl::Context get_context() const;

    const cl::Buffer& get_voxel_index_buffer() const;
    aabb get_global_aabb() const;
    cl_uint get_side() const;

    const cl::Buffer& get_triangles_buffer() const;
    const cl::Buffer& get_vertices_buffer() const;
    const cl::Buffer& get_surfaces_buffer() const;

private:
    const cl::Context context;

    const cl::Buffer voxel_index_buffer;
    const aabb global_aabb;
    const cl_uint side;

    const cl::Buffer triangles_buffer;
    const cl::Buffer vertices_buffer;
    const cl::Buffer surfaces_buffer;
};
