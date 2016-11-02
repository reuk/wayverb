#include "waveguide/mesh.h"
#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/fitted_boundary.h"
#include "waveguide/mesh_setup_program.h"
#include "waveguide/program.h"

#include "core/conversions.h"
#include "core/scene_data_loader.h"
#include "core/spatial_division/scene_buffers.h"
#include "core/spatial_division/voxelised_scene_data.h"

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

void mesh::set_coefficients(coefficients_canonical coefficients) {
    vectors_.set_coefficients(coefficients);
}

void mesh::set_coefficients(
        util::aligned::vector<coefficients_canonical> coefficients) {
    vectors_.set_coefficients(coefficients);
}

////////////////////////////////////////////////////////////////////////////////

mesh compute_mesh(
        const core::compute_context& cc,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised,
        float mesh_spacing,
        float speed_of_sound) {
    const auto program = setup_program{cc};
    auto queue = cl::CommandQueue{cc.context, cc.device};

    const auto buffers = make_scene_buffers(cc.context, voxelised);

    const auto desc = [&] {
        const auto aabb = voxelised.get_voxels().get_aabb();
        const auto dim = glm::ivec3{dimensions(aabb) / mesh_spacing};
        return mesh_descriptor{core::to_cl_float3(aabb.get_min()),
                               core::to_cl_int3(dim),
                               mesh_spacing};
    }();

    auto nodes = [&] {
        const auto num_nodes = compute_num_nodes(desc);

        cl::Buffer node_buffer{cc.context,
                               CL_MEM_READ_WRITE,
                               num_nodes * sizeof(condensed_node)};

        const auto enqueue = [&] {
            return cl::EnqueueArgs(queue, cl::NDRange(num_nodes));
        };

        //  find whether each node is inside or outside the model
        {
            auto kernel = program.get_node_inside_kernel();
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
            auto nodes =
                    core::read_from_buffer<condensed_node>(queue, node_buffer);
            const auto count =
                    count_boundary_type(nodes.begin(), nodes.end(), [](auto i) {
                        return i == id_inside;
                    });
            if (!count) {
                throw std::runtime_error("no inside nodes found");
            }
        }
#endif

        //  find node boundary type
        {
            auto kernel = program.get_node_boundary_kernel();
            kernel(enqueue(), node_buffer, desc);
        }

        return core::read_from_buffer<condensed_node>(queue, node_buffer);
    }();

    //  IMPORTANT
    //  compute_boundary_index_data mutates the nodes array, so it must
    //  be run before condensing the nodes.
    auto boundary_data =
            compute_boundary_index_data(cc.device, buffers, desc, nodes);

    auto v = vectors{
            std::move(nodes),
            util::map_to_vector(
                    begin(voxelised.get_scene_data().get_surfaces()),
                    end(voxelised.get_scene_data().get_surfaces()),
                    [&](const auto& surface) {
                        return to_impedance_coefficients(
                                compute_reflectance_filter_coefficients(
                                        surface.absorption.s,
                                        1 / waveguide::config::time_step(
                                                    speed_of_sound,
                                                    mesh_spacing)));
                    }),
            std::move(boundary_data)};

    return {desc, std::move(v)};
}

voxels_and_mesh compute_voxels_and_mesh(
        const core::compute_context& cc,
        const core::generic_scene_data<cl_float3,
                                       core::surface<core::simulation_bands>>&
                scene,
        const glm::vec3& anchor,
        double sample_rate,
        double speed_of_sound) {
    const auto mesh_spacing =
            waveguide::config::grid_spacing(speed_of_sound, 1 / sample_rate);
    auto voxelised = make_voxelised_scene_data(
            scene,
            5,
            waveguide::compute_adjusted_boundary(
                    core::geo::get_aabb(scene), anchor, mesh_spacing));
    auto mesh = compute_mesh(cc, voxelised, mesh_spacing, speed_of_sound);
    return {std::move(voxelised), std::move(mesh)};
}

}  // namespace waveguide
