#pragma once

#include "waveguide/mesh_descriptor.h"
#include "waveguide/setup.h"

#include "core/spatial_division/voxelised_scene_data.h"

namespace waveguide {

class mesh final {
public:
    mesh(mesh_descriptor descriptor, vectors vectors);

    const mesh_descriptor& get_descriptor() const;
    const vectors& get_structure() const;

    void set_coefficients(coefficients_canonical coefficients);
    void set_coefficients(
            util::aligned::vector<coefficients_canonical> coefficients);

private:
    mesh_descriptor descriptor_;
    vectors vectors_;
};

bool is_inside(const mesh& m, size_t node_index);

///  use this if you already have a voxelised scene
mesh compute_mesh(
        const core::compute_context& cc,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised,
        float mesh_spacing,
        float speed_of_sound);

struct voxels_and_mesh final {
    core::voxelised_scene_data<cl_float3, core::surface<core::simulation_bands>>
            voxels;
    mesh mesh;
};

/// this one should be prefered - will set up a voxelised scene with the correct
/// boundaries, and then will use it to create a mesh
voxels_and_mesh compute_voxels_and_mesh(
        const core::compute_context& cc,
        const core::generic_scene_data<cl_float3,
                                       core::surface<core::simulation_bands>>&
                scene,
        const glm::vec3& anchor,  //  probably the receiver if you want it to
                                  //  coincide with an actual node
        double sample_rate,
        double speed_of_sound);

}  // namespace waveguide
