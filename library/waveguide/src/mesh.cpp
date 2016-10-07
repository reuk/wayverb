#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/mesh.h"
#include "waveguide/mesh_setup_program.h"
#include "waveguide/program.h"
#include "waveguide/surface_filters.h"

#include "common/conversions.h"
#include "common/scene_data_loader.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "utilities/popcount.h"

#include <iostream>

namespace waveguide {

mesh::mesh(mesh_descriptor descriptor, vectors vectors)
        : descriptor_(std::move(descriptor))
        , vectors_(std::move(vectors)) {}

const mesh_descriptor& mesh::get_descriptor() const { return descriptor_; }
const vectors& mesh::get_structure() const { return vectors_; }

bool is_inside(const mesh& m, size_t node_index) {
    return is_inside(m.get_structure().get_condensed_nodes()[node_index]);
}

void mesh::set_coefficients(
        aligned::vector<coefficients_canonical> coefficients) {
    vectors_.set_coefficients(coefficients);
}

//----------------------------------------------------------------------------//

mesh compute_mesh(const compute_context& cc,
                  const voxelised_scene_data<cl_float3, surface>& voxelised,
                  float mesh_spacing,
                  float speed_of_sound) {
    const setup_program program{cc};
    cl::CommandQueue queue{cc.context, cc.device};

    const auto buffers{make_scene_buffers(cc.context, voxelised)};

    const auto desc{[&] {
        const auto aabb{voxelised.get_voxels().get_aabb()};
        const auto dim{glm::ivec3{dimensions(aabb) / mesh_spacing}};
        return mesh_descriptor{
                to_cl_float3(aabb.get_min()), to_cl_int3(dim), mesh_spacing};
    }()};

    auto nodes{[&] {
        const auto num_nodes{compute_num_nodes(desc)};

        cl::Buffer node_buffer{cc.context,
                               CL_MEM_READ_WRITE,
                               num_nodes * sizeof(condensed_node)};

        const auto enqueue{
                [&] { return cl::EnqueueArgs(queue, cl::NDRange(num_nodes)); }};

        //  find whether each node is inside or outside the model
        {
            auto kernel{program.get_node_inside_kernel()};
            kernel(enqueue(),
                   node_buffer,
                   desc,
                   buffers.get_voxel_index_buffer(),
                   buffers.get_global_aabb(),
                   buffers.get_side(),
                   buffers.get_triangles_buffer(),
                   buffers.get_vertices_buffer());
        }

#ifndef NDEBUG
        {
            auto nodes{read_from_buffer<condensed_node>(queue, node_buffer)};
            const auto count{
                    count_boundary_type(nodes.begin(), nodes.end(), [](auto i) {
                        return i == id_inside;
                    })};
            if (!count) {
                throw std::runtime_error("no inside nodes found");
            }
        }
#endif

        //  find node boundary type
        {
            auto kernel{program.get_node_boundary_kernel()};
            kernel(enqueue(), node_buffer, desc);
        }

        return read_from_buffer<condensed_node>(queue, node_buffer);
    }()};

    //  IMPORTANT
    //  compute_boundary_index_data mutates the nodes array, so it must
    //  be run before condensing the nodes.
    auto boundary_data{
            compute_boundary_index_data(cc.device, buffers, desc, nodes)};

    //  TODO Use appropriate method for finding filter coefficients.
    auto v{vectors{
            std::move(nodes),
            to_flat_coefficients(voxelised.get_scene_data().get_surfaces()),
            std::move(boundary_data)}};

    return {desc, std::move(v)};
}

std::tuple<voxelised_scene_data<cl_float3, surface>, mesh>
compute_voxels_and_mesh(const compute_context& cc,
                        const generic_scene_data<cl_float3, surface>& scene,
                        const glm::vec3& anchor,
                        double sample_rate,
                        double speed_of_sound) {
    const auto mesh_spacing{
            waveguide::config::grid_spacing(speed_of_sound, 1 / sample_rate)};
    auto voxelised{make_voxelised_scene_data(
            scene,
            5,
            waveguide::compute_adjusted_boundary(
                    geo::get_aabb(scene), anchor, mesh_spacing))};
    auto mesh{compute_mesh(cc, voxelised, mesh_spacing, speed_of_sound)};
    return std::make_tuple(std::move(voxelised), std::move(mesh));
}

}  // namespace waveguide
