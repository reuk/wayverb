#pragma once

#include "core/cl/voxel_structs.h"
#include "core/spatial_division/voxelised_scene_data.h"

namespace wayverb {
namespace core {

/// Provides a simple utility for loading voxelised scene data to the gpu in
/// one go.
template <typename Vertex, typename Surface>
class generic_scene_buffers final {
public:
    generic_scene_buffers(
            const cl::Context& context,
            const voxelised_scene_data<Vertex, Surface>& scene_data)
            : context_{context}
            , voxel_index_{load_to_buffer(
                      context_, get_flattened(scene_data.get_voxels()), true)}
            , global_aabb_{to_cl_float3{}(scene_data.get_voxels()
                                                  .get_aabb()
                                                  .get_min()),
                           to_cl_float3{}(scene_data.get_voxels()
                                                  .get_aabb()
                                                  .get_max())}
            , side_{static_cast<cl_uint>(scene_data.get_voxels().get_side())}
            , triangles_{load_to_buffer(
                      context_,
                      scene_data.get_scene_data().get_triangles(),
                      true)}
            , vertices_{load_to_buffer(
                      context_,
                      scene_data.get_scene_data().get_vertices(),
                      true)}
            , surfaces_{
                      load_to_buffer(context_,
                                     scene_data.get_scene_data().get_surfaces(),
                                     true)} {}

    cl::Context get_context() const { return context_; }

    const cl::Buffer& get_voxel_index_buffer() const { return voxel_index_; }
    aabb get_global_aabb() const { return global_aabb_; }
    cl_uint get_side() const { return side_; }

    const cl::Buffer& get_triangles_buffer() const { return triangles_; }
    const cl::Buffer& get_vertices_buffer() const { return vertices_; }
    const cl::Buffer& get_surfaces_buffer() const { return surfaces_; }

private:
    const cl::Context context_;

    const cl::Buffer voxel_index_;
    const aabb global_aabb_;
    const cl_uint side_;

    const cl::Buffer triangles_;
    const cl::Buffer vertices_;
    const cl::Buffer surfaces_;
};

template <typename Vertex, typename Surface>
auto make_scene_buffers(
        const cl::Context& context,
        const voxelised_scene_data<Vertex, Surface>& scene_data) {
    return generic_scene_buffers<Vertex, Surface>{context, scene_data};
}

using scene_buffers =
        generic_scene_buffers<cl_float3, surface<simulation_bands>>;

}  // namespace core
}  // namespace wayverb
