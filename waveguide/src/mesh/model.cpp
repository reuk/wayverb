#include "waveguide/mesh/model.h"
#include "waveguide/config.h"
#include "waveguide/mesh/boundary_adjust.h"
#include "waveguide/program.h"
#include "waveguide/surface_filters.h"

#include "common/conversions.h"
#include "common/popcount.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include <iostream>

namespace waveguide {
namespace mesh {

model::model(const struct descriptor& descriptor,
             const class vectors& vectors,
             const aligned::vector<glm::vec3>& node_positions)
        : descriptor(descriptor)
        , vectors(vectors)
        , node_positions(node_positions) {}

const descriptor& model::get_descriptor() const { return descriptor; }
const vectors& model::get_structure() const { return vectors; }
const aligned::vector<glm::vec3>& model::get_node_positions() const {
    return node_positions;
}

bool is_inside(const model& m, size_t node_index) {
    return is_inside(m.get_structure().get_condensed_nodes()[node_index]);
}

//----------------------------------------------------------------------------//

std::tuple<aligned::vector<node>, descriptor> compute_fat_nodes(
        const cl::Context& context,
        const cl::Device& device,
        const voxelised_scene_data& voxelised,
        const scene_buffers& buffers,
        float mesh_spacing) {
    const auto program{setup_program{context, device}};
    auto queue{cl::CommandQueue{context, device}};

    const auto desc{[&] {
        const auto aabb{voxelised.get_voxels().get_aabb()};
        const auto dim{glm::ivec3{dimensions(aabb) / mesh_spacing}};
        return descriptor{aabb.get_min(), dim, mesh_spacing};
    }()};

    const auto num_nodes{desc.dimensions.x * desc.dimensions.y *
                         desc.dimensions.z};
    auto node_buffer{
            cl::Buffer{context, CL_MEM_READ_WRITE, num_nodes * sizeof(node)}};

    const auto enqueue{
            [&] { return cl::EnqueueArgs(queue, cl::NDRange(num_nodes)); }};

    //  find where each node is, and where its neighbors are
    {
        auto kernel = program.get_node_position_and_neighbors_kernel();
        kernel(enqueue(),
               node_buffer,
               to_cl_int3(desc.dimensions),
               to_cl_float3(desc.min_corner),
               desc.spacing);
    }

    //  find whether each node is inside or outside the model
    {
        auto kernel = program.get_node_inside_kernel();
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
        auto kernel = program.get_node_boundary_type_kernel();
        kernel(enqueue(), node_buffer, to_cl_int3(desc.dimensions));
    }

    //  return results
    return std::make_tuple(read_from_buffer<node>(queue, node_buffer), desc);
}

model compute_model(const cl::Context& context,
                    const cl::Device& device,
                    const voxelised_scene_data& voxelised,
                    double mesh_spacing,
                    double speed_of_sound) {
    const auto buffers{scene_buffers{context, voxelised}};

    aligned::vector<node> nodes;
    descriptor desc;
    std::tie(nodes, desc) = compute_fat_nodes(
            context, device, voxelised, buffers, mesh_spacing);

    const auto node_positions{map_to_vector(
            nodes, [](const auto& i) { return to_vec3(i.position); })};

    const auto sample_rate{1 / config::time_step(speed_of_sound, mesh_spacing)};

    aligned::vector<boundary_index_array_1> bia_1;
    aligned::vector<boundary_index_array_2> bia_2;
    aligned::vector<boundary_index_array_3> bia_3;
    std::tie(bia_1, bia_2, bia_3) =
            compute_boundary_index_data(device, buffers, nodes, desc);

    const auto v{vectors{
            get_condensed(nodes),
            to_filter_coefficients(voxelised.get_scene_data().get_surfaces(),
                                   sample_rate),
            std::move(bia_1),
            std::move(bia_2),
            std::move(bia_3)}};

    return model{desc, v, node_positions};
}

std::tuple<voxelised_scene_data, model> compute_voxels_and_model(
        const cl::Context& context,
        const cl::Device& device,
        const copyable_scene_data& scene,
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
    auto model{compute_model(
            context, device, voxelised, mesh_spacing, speed_of_sound)};
    return std::make_tuple(std::move(voxelised), std::move(model));
}

}  // namespace mesh
}  // namespace waveguide
