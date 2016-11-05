#pragma once

#include "raytracer/image_source/reflection_path_builder.h"

#include "core/cl/common.h"
#include "core/environment.h"
#include "core/spatial_division/scene_buffers.h"

namespace wayverb {
namespace raytracer {
namespace reflection_processor {

class image_source final {
public:
    image_source(size_t max_order,
                 bool flip_phase,
                 const glm::vec3& source,
                 const glm::vec3& receiver,
                 const core::environment& environment,
                 const core::voxelised_scene_data<
                         cl_float3,
                         core::surface<core::simulation_bands>>& voxelised,
                 size_t items);

    template <typename It>
    void process(It b,
                 It e,
                 const core::scene_buffers& buffers,
                 size_t step,
                 size_t total) {
        if (step < max_image_source_order_) {
            builder_.push(b, e);
        }
    }

    util::aligned::vector<impulse<8>> get_results();

private:
    glm::vec3 source_;
    glm::vec3 receiver_;
    core::environment environment_;
    const core::voxelised_scene_data<cl_float3,
                                     core::surface<core::simulation_bands>>&
            voxelised_;
    size_t max_image_source_order_;
    bool flip_phase_;

    raytracer::image_source::reflection_path_builder builder_;
};

class make_image_source final {
public:
    make_image_source(size_t max_order, bool flip_phase);

    image_source operator()(
            const core::compute_context& cc,
            const glm::vec3& source,
            const glm::vec3& receiver,
            const core::environment& environment,
            const core::voxelised_scene_data<
                    cl_float3,
                    core::surface<core::simulation_bands>>& voxelised,
            size_t num_directions) const;

private:
    size_t max_order_;
    bool flip_phase_;
};

}  // namespace reflection_processor
}  // namespace raytracer
}  // namespace wayverb
