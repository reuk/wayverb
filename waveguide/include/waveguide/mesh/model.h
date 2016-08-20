#pragma once

#include "waveguide/mesh/descriptor.h"
#include "waveguide/mesh/setup.h"

class voxelised_scene_data;

namespace waveguide {
namespace mesh {

class model final {
public:
    model(const struct descriptor& descriptor,
          const vectors& vectors,
          const aligned::vector<glm::vec3>& node_positions);

    const descriptor& get_descriptor() const;
    const vectors& get_structure() const;
    const aligned::vector<glm::vec3>& get_node_positions() const;

private:
    struct descriptor descriptor;
    vectors vectors;
    aligned::vector<glm::vec3> node_positions;
};

bool is_inside(const model& m, size_t node_index);

std::tuple<aligned::vector<node>, descriptor> compute_fat_nodes(
        const cl::Context&,
        const cl::Device&,
        const voxelised_scene_data& voxelised,
        const scene_buffers& buffers,
        float mesh_spacing);

///  use this if you already have a voxelised scene
model compute_model(const cl::Context& context,
                    const cl::Device& device,
                    const voxelised_scene_data& voxelised,
                    double mesh_spacing,
                    double speed_of_sound);

/// this one should be prefered - will set up a voxelised scene with the correct
/// boundaries, and then will use it to create a mesh
std::tuple<voxelised_scene_data, model> compute_voxels_and_model(
        const cl::Context& context,
        const cl::Device& device,
        const copyable_scene_data& scene,
        const glm::vec3& anchor,  //  probably the receiver if you want it to
                                  //  coincide with an actual node
        double sample_rate,
        double speed_of_sound);

}  // namespace mesh
}  // namespace waveguide
