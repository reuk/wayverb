#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/mesh.h"
#include "waveguide/program.h"
#include "waveguide/surface_filters.h"

#include "common/conversions.h"
#include "common/popcount.h"
#include "common/scene_data_loader.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include <iostream>

namespace waveguide {

mesh::mesh(descriptor descriptor, vectors vectors)
        : descriptor_(std::move(descriptor))
        , vectors_(std::move(vectors)) {}

const descriptor& mesh::get_descriptor() const { return descriptor_; }
const vectors& mesh::get_structure() const { return vectors_; }

bool is_inside(const mesh& m, size_t node_index) {
    return is_inside(m.get_structure().get_condensed_nodes()[node_index]);
}

void mesh::set_coefficients(
        aligned::vector<coefficients_canonical> coefficients) {
    vectors_.set_coefficients(coefficients);
}

//----------------------------------------------------------------------------//

fat_nodes compute_fat_nodes(const compute_context& cc,
                            const voxelised_scene_data& voxelised,
                            const scene_buffers& buffers,
                            float mesh_spacing) {
    const setup_program program{cc};
    cl::CommandQueue queue{cc.context, cc.device};

    const auto desc{[&] {
        const auto aabb{voxelised.get_voxels().get_aabb()};
        const auto dim{glm::ivec3{dimensions(aabb) / mesh_spacing}};
        return descriptor{aabb.get_min(), dim, mesh_spacing};
    }()};

    const auto num_nodes{desc.dimensions.x * desc.dimensions.y *
                         desc.dimensions.z};
    cl::Buffer node_buffer{
            cc.context, CL_MEM_READ_WRITE, num_nodes * sizeof(node)};

    const auto enqueue{
            [&] { return cl::EnqueueArgs(queue, cl::NDRange(num_nodes)); }};

    //  find where each node is, and where its neighbors are
    {
        auto kernel{program.get_node_position_and_neighbors_kernel()};
        kernel(enqueue(),
               node_buffer,
               to_cl_int3(desc.dimensions),
               to_cl_float3(desc.min_corner),
               desc.spacing);
    }

    //  find whether each node is inside or outside the model
    {
        auto kernel{program.get_node_inside_kernel()};
        kernel(enqueue(),
               node_buffer,
               buffers.get_voxel_index_buffer(),
               buffers.get_global_aabb(),
               buffers.get_side(),
               buffers.get_triangles_buffer(),
               buffers.get_vertices_buffer());
    }

    //  find node boundary type
    {
        auto kernel{program.get_node_boundary_type_kernel()};
        kernel(enqueue(), node_buffer, to_cl_int3(desc.dimensions));
    }

    //  return results
    return {desc, read_from_buffer<node>(queue, node_buffer)};
}

mesh compute_mesh(const compute_context& cc,
                  const voxelised_scene_data& voxelised,
                  double mesh_spacing,
                  double speed_of_sound) {
    const scene_buffers buffers{cc.context, voxelised};

    auto fat_nodes{compute_fat_nodes(cc, voxelised, buffers, mesh_spacing)};

    const auto sample_rate{1 / config::time_step(speed_of_sound, mesh_spacing)};

    //  IMPORTANT
    //  compute_boundary_index_data mutates the nodes array, so it must
    //  be run before condensing the nodes.
    auto boundary_data{compute_boundary_index_data(
            cc.device, buffers, fat_nodes.nodes, fat_nodes.descriptor)};

    auto v{vectors{
            get_condensed(fat_nodes.nodes),
            to_filter_coefficients(voxelised.get_scene_data().get_surfaces(),
                                   sample_rate),
            std::move(boundary_data)}};

    return {fat_nodes.descriptor, std::move(v)};
}

std::tuple<voxelised_scene_data, mesh> compute_voxels_and_mesh(
        const compute_context& cc,
        const scene_data& scene,
        const glm::vec3& anchor,
        double sample_rate,
        double speed_of_sound) {
    const auto mesh_spacing{
            waveguide::config::grid_spacing(speed_of_sound, 1 / sample_rate)};
    auto voxelised{voxelised_scene_data{
            scene,
            5,
            waveguide::compute_adjusted_boundary(
                    scene.get_aabb(), anchor, mesh_spacing)}};
    auto mesh{compute_mesh(cc, voxelised, mesh_spacing, speed_of_sound)};
    return std::make_tuple(std::move(voxelised), std::move(mesh));
}

}  // namespace waveguide
