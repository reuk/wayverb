#include "common/spatial_division/scene_buffers.h"
#include "common/conversions.h"
#include "common/spatial_division/voxelised_scene_data.h"

scene_buffers::scene_buffers(const cl::Context& context,
                             const voxelised_scene_data& scene_data)
        : context(context)
        , voxel_index_buffer(load_to_buffer(
                  context, get_flattened(scene_data.get_voxels()), true))
        , global_aabb(aabb{
                  to_cl_float3(scene_data.get_voxels().get_aabb().get_min()),
                  to_cl_float3(scene_data.get_voxels().get_aabb().get_max())})
        , side(scene_data.get_voxels().get_side())
        , triangles_buffer(load_to_buffer(
                  context, scene_data.get_scene_data().get_triangles(), true))
        , vertices_buffer(load_to_buffer(
                  context, scene_data.get_scene_data().get_vertices(), true))
        , surfaces_buffer(load_to_buffer(
                  context, scene_data.get_scene_data().get_surfaces(), true)) {}

cl::Context scene_buffers::get_context() const { return context; }

const cl::Buffer& scene_buffers::get_voxel_index_buffer() const {
    return voxel_index_buffer;
}

aabb scene_buffers::get_global_aabb() const { return global_aabb; }

cl_uint scene_buffers::get_side() const { return side; }

const cl::Buffer& scene_buffers::get_triangles_buffer() const {
    return triangles_buffer;
}

const cl::Buffer& scene_buffers::get_vertices_buffer() const {
    return vertices_buffer;
}

const cl::Buffer& scene_buffers::get_surfaces_buffer() const {
    return surfaces_buffer;
}
