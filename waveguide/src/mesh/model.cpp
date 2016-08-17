#include "waveguide/config.h"
#include "waveguide/mesh/boundary_adjust.h"
#include "waveguide/mesh/model.h"
#include "waveguide/program.h"
#include "waveguide/surface_filters.h"

#include "common/conversions.h"
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
        float mesh_spacing) {
    setup_program program(context, device);
    cl::CommandQueue queue(context, device);

    const auto desc = [&] {
        const auto aabb = voxelised.get_voxels().get_aabb();
        const glm::ivec3 dim = dimensions(aabb) / mesh_spacing;
        return descriptor{aabb.get_min(), dim, mesh_spacing};
    }();

    const auto num_nodes =
            desc.dimensions.x * desc.dimensions.y * desc.dimensions.z;
    cl::Buffer node_buffer(
            context, CL_MEM_READ_WRITE, num_nodes * sizeof(node));

    //  find where each node is, and where its neighbors are
    {
        auto kernel = program.get_node_position_and_neighbors_kernel();
        kernel(cl::EnqueueArgs(queue, cl::NDRange(num_nodes)),
               node_buffer,
               to_cl_int3(desc.dimensions),
               to_cl_float3(desc.min_corner),
               desc.spacing);
    }

    const scene_buffers buffers(context, voxelised);

    //  find whether each node is inside or outside the model
    {
        auto kernel = program.get_node_inside_kernel();
        kernel(cl::EnqueueArgs(queue, cl::NDRange(num_nodes)),
               node_buffer,
               buffers.get_voxel_index_buffer(),
               buffers.get_global_aabb(),
               buffers.get_side(),
               buffers.get_triangles_buffer(),
               buffers.get_vertices_buffer());
    }

    //  find node boundary type
    {

    }

    //  find node boundary index
    {}

    //  return results
    return std::make_tuple(read_from_buffer<node>(queue, node_buffer), desc);
}

model compute_model(const cl::Context& context,
                    const cl::Device& device,
                    const voxelised_scene_data& voxelised,
                    float mesh_spacing) {
    aligned::vector<node> nodes;
    descriptor desc;
    std::tie(nodes, desc) =
            compute_fat_nodes(context, device, voxelised, mesh_spacing);

    const auto node_positions = map_to_vector(
            nodes, [](const auto& i) { return to_vec3(i.position); });

    const auto sample_rate =
            1 / config::time_step(speed_of_sound, mesh_spacing);

    vectors vectors(
            nodes,
            to_filter_coefficients(voxelised.get_scene_data().get_surfaces(),
                                   sample_rate));

    return model(desc, vectors, node_positions);
}

}  // namespace mesh
}  // namespace waveguide
