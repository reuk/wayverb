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
        float mesh_spacing);

model compute_model(const cl::Context& context,
                    const cl::Device& device,
                    const voxelised_scene_data& voxelised,
                    float mesh_spacing);

}  // namespace mesh
}  // namespace waveguide
