#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/iterative_builder.h"

#include "core/cl/common.h"
#include "core/environment.h"
#include "core/spatial_division/scene_buffers.h"

namespace wayverb {
namespace raytracer {
namespace reflection_processor {

class visual final {
public:
    explicit visual(size_t items);

    template <typename It>
    void process(It b,
                 It e,
                 const core::scene_buffers& buffers,
                 size_t step,
                 size_t total) {
        builder_.push(b, b + builder_.get_num_items());
    }

    util::aligned::vector<util::aligned::vector<reflection>> get_results();

private:
    iterative_builder<reflection> builder_;
};

class make_visual final {
public:
    explicit make_visual(size_t items);

    visual operator()(const core::compute_context& cc,
                      const glm::vec3& source,
                      const glm::vec3& receiver,
                      const core::environment& environment,
                      const core::voxelised_scene_data<
                              cl_float3,
                              core::surface<core::simulation_bands>>& voxelised,
                      size_t num_directions) const;

private:
    size_t items_;
};

}  // namespace reflection_processor
}  // namespace raytracer
}  // namespace wayverb
