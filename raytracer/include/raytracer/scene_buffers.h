#pragma once

#include "raytracer/cl_structs.h"
#include "raytracer/raytracer_program.h"

#include "common/aligned/vector.h"
#include "common/cl_common.h"
#include "common/voxel_collection.h"

class CopyableSceneData;

namespace raytracer {

class scene_buffers final {
public:
    scene_buffers(const cl::Context&,
                  const cl::Device&,
                  const CopyableSceneData& scene_data);
    scene_buffers(const cl::Context&,
                  const cl::Device&,
                  const CopyableSceneData& scene_data,
                  const VoxelCollection& voxel_collection);

    const cl::Buffer& get_voxel_index_buffer() const;
    AABB get_global_aabb() const;
    cl_ulong get_side() const;

    const cl::Buffer& get_triangles_buffer() const;
    const cl::Buffer& get_vertices_buffer() const;
    const cl::Buffer& get_surfaces_buffer() const;

    cl::CommandQueue& get_queue();
    const cl::CommandQueue& get_queue() const;

private:
    cl::CommandQueue queue;

    const cl::Buffer voxel_index_buffer;
    const AABB global_aabb;
    const cl_ulong side;

    const cl::Buffer triangles_buffer;
    const cl::Buffer vertices_buffer;
    const cl::Buffer surfaces_buffer;
};

}  // namespace raytracer
