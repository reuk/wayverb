#include "waveguide/mesh/model.h"
#include "waveguide/config.h"
#include "waveguide/mesh/boundary_adjust.h"
#include "waveguide/program.h"
#include "waveguide/surface_filters.h"

#include "common/voxelised_scene_data.h"
#include "common/conversions.h"

namespace waveguide {
namespace mesh {

model::model(const struct descriptor& descriptor,
             const setup::vectors& vectors,
             const aligned::vector<glm::vec3>& node_positions)
        : descriptor(descriptor)
        , vectors(vectors)
        , node_positions(node_positions) {}

const descriptor& model::get_descriptor() const { return descriptor; }
const setup::vectors& model::get_structure() const { return vectors; }
const aligned::vector<glm::vec3>& model::get_node_positions() const {
    return node_positions;
}

//----------------------------------------------------------------------------//

std::tuple<aligned::vector<setup::node>, descriptor> compute_fat_nodes(
        const cl::Context& context,
        const cl::Device& device,
        const voxelised_scene_data& voxelised,
        float mesh_spacing) {
    setup::setup_program program(context, device);
    cl::CommandQueue queue(context, device);

    const auto desc = [&] {
        const auto aabb = voxelised.get_voxels().get_aabb();
        const glm::ivec3 dim = dimensions(aabb) / mesh_spacing;
        return descriptor{aabb.get_min(), dim, mesh_spacing};
    }();

    const auto num_nodes =
            desc.dimensions.x * desc.dimensions.y * desc.dimensions.z;
    cl::Buffer node_buffer(
            context, CL_MEM_READ_WRITE, num_nodes * sizeof(setup::node));

    {
        auto kernel = program.get_node_position_and_neighbors_kernel();
        kernel(cl::EnqueueArgs(queue, cl::NDRange(num_nodes)),
               node_buffer,
               to_cl_int3(desc.dimensions),
               to_cl_float3(desc.min_corner),
               desc.spacing);
    }

    return std::make_tuple(read_from_buffer<setup::node>(queue, node_buffer),
                           desc);
}

model compute_model(const cl::Context& context,
                    const cl::Device& device,
                    const voxelised_scene_data& voxelised,
                    float mesh_spacing) {
    aligned::vector<setup::node> nodes;
    descriptor desc;
    std::tie(nodes, desc) =
            compute_fat_nodes(context, device, voxelised, mesh_spacing);

    const auto node_positions = map_to_vector(
            nodes, [](const auto& i) { return to_vec3(i.position); });

    const auto sample_rate =
            1 / config::time_step(speed_of_sound, mesh_spacing);

    setup::vectors vectors(
            nodes,
            filters::to_filter_coefficients(
                    voxelised.get_scene_data().get_surfaces(), sample_rate));

    return model(desc, vectors, node_positions);
}

}  // namespace mesh
}  // namespace waveguide
