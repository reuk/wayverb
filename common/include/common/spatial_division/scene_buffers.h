#pragma once

#include "common/cl/voxel.h"

class voxelised_scene_data;

/// Provides a simple utility for loading voxelised scene data to the gpu in
/// one go.
class scene_buffers final {
public:
    scene_buffers(const cl::Context&, const voxelised_scene_data& scene_data);

    const cl::Buffer& get_voxel_index_buffer() const;
    aabb get_global_aabb() const;
    cl_uint get_side() const;

    const cl::Buffer& get_triangles_buffer() const;
    const cl::Buffer& get_vertices_buffer() const;
    const cl::Buffer& get_surfaces_buffer() const;

private:
    const cl::Buffer voxel_index_buffer;
    const aabb global_aabb;
    const cl_uint side;

    const cl::Buffer triangles_buffer;
    const cl::Buffer vertices_buffer;
    const cl::Buffer surfaces_buffer;
};
