#pragma once

#include "waveguide/descriptor.h"
#include "waveguide/setup.h"

#include "common/spatial_division/voxelised_scene_data.h"

namespace waveguide {

class mesh final {
public:
    mesh(descriptor descriptor, vectors vectors);

    const descriptor& get_descriptor() const;
    const vectors& get_structure() const;

    void set_coefficients(aligned::vector<coefficients_canonical> coefficients);

private:
    descriptor descriptor_;
    vectors vectors_;
};

bool is_inside(const mesh& m, size_t node_index);

struct fat_nodes final {
    descriptor descriptor;
    aligned::vector<node> nodes;
};

fat_nodes compute_fat_nodes(const compute_context& cc,
                            const voxelised_scene_data& voxelised,
                            const scene_buffers& buffers,
                            float mesh_spacing);

///  use this if you already have a voxelised scene
mesh compute_mesh(const compute_context& cc,
                  const voxelised_scene_data& voxelised,
                  double mesh_spacing,
                  double speed_of_sound);

/// this one should be prefered - will set up a voxelised scene with the correct
/// boundaries, and then will use it to create a mesh
std::tuple<voxelised_scene_data, mesh> compute_voxels_and_mesh(
        const compute_context& cc,
        const scene_data& scene,
        const glm::vec3& anchor,  //  probably the receiver if you want it to
                                  //  coincide with an actual node
        double sample_rate,
        double speed_of_sound);

}  // namespace waveguide
