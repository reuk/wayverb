#pragma once

#include "raytracer/cl_structs.h"

#include "common/aligned/vector.h"
#include "common/cl_common.h"
#include "common/voxel_collection.h"

class copyable_scene_data;

namespace raytracer {

class scene_buffers final {
public:
    scene_buffers(const cl::Context&,
                  const cl::Device&,
                  const copyable_scene_data& scene_data,
                  const voxel_collection<3>& vox);
    const cl::Buffer& get_voxel_index_buffer() const;
    aabb get_global_aabb() const;
    cl_ulong get_side() const;

    const cl::Buffer& get_triangles_buffer() const;
    const cl::Buffer& get_vertices_buffer() const;
    const cl::Buffer& get_surfaces_buffer() const;

    cl::CommandQueue& get_queue();
    const cl::CommandQueue& get_queue() const;

private:
    cl::CommandQueue queue;

    const cl::Buffer voxel_index_buffer;
    const aabb global_aabb;
    const cl_ulong side;

    const cl::Buffer triangles_buffer;
    const cl::Buffer vertices_buffer;
    const cl::Buffer surfaces_buffer;
};

}  // namespace raytracer
