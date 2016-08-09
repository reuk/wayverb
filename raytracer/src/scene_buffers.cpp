#include "raytracer/scene_buffers.h"

#include "common/conversions.h"

namespace raytracer {

scene_buffers::scene_buffers(const cl::Context& context,
                             const cl::Device& device,
                             const copyable_scene_data& scene_data,
                             const voxel_collection<3>& vox)
        : queue(context, device)
        , voxel_index_buffer(load_to_buffer(context, get_flattened(vox), true))
        , global_aabb(aabb{to_cl_float3(vox.get_aabb().get_min()),
                           to_cl_float3(vox.get_aabb().get_max())})
        , side(vox.get_side())
        , triangles_buffer(
                  load_to_buffer(context, scene_data.get_triangles(), true))
        , vertices_buffer(
                  load_to_buffer(context, scene_data.get_vertices(), true))
        , surfaces_buffer(
                  load_to_buffer(context, scene_data.get_surfaces(), true)) {}

const cl::Buffer& scene_buffers::get_voxel_index_buffer() const {
    return voxel_index_buffer;
}

aabb scene_buffers::get_global_aabb() const { return global_aabb; }

cl_ulong scene_buffers::get_side() const { return side; }

const cl::Buffer& scene_buffers::get_triangles_buffer() const {
    return triangles_buffer;
}

const cl::Buffer& scene_buffers::get_vertices_buffer() const {
    return vertices_buffer;
}

const cl::Buffer& scene_buffers::get_surfaces_buffer() const {
    return surfaces_buffer;
}

cl::CommandQueue& scene_buffers::get_queue() { return queue; }
const cl::CommandQueue& scene_buffers::get_queue() const { return queue; }

}  // namespace raytracer
