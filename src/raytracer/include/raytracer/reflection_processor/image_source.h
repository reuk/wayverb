#pragma once

#include "raytracer/image_source/reflection_path_builder.h"

#include "core/cl/common.h"
#include "core/environment.h"
#include "core/spatial_division/scene_buffers.h"

namespace wayverb {
namespace raytracer {
namespace reflection_processor {

class image_source_group_processor final {
public:
    image_source_group_processor(size_t max_order, size_t items);

    template <typename It>
    void process(It b,
                 It e,
                 const core::scene_buffers& /*buffers*/,
                 size_t step,
                 size_t /*total*/) {
        if (step < max_image_source_order_) {
            builder_.push(b, e);
        }
    }

    auto get_results() const { return builder_.get_data(); }

private:
    size_t max_image_source_order_;

    raytracer::image_source::reflection_path_builder builder_;
};

////////////////////////////////////////////////////////////////////////////////

class image_source_processor final {
public:
    image_source_processor(
            const glm::vec3& source,
            const glm::vec3& receiver,
            const core::environment& environment,
            const core::voxelised_scene_data<
                    cl_float3,
                    core::surface<core::simulation_bands>>& voxelised,
            size_t max_order);

    image_source_group_processor get_group_processor(
            size_t num_directions) const;
    void accumulate(const image_source_group_processor& processor);

    util::aligned::vector<impulse<8>> get_results() const;

private:
    glm::vec3 source_;
    glm::vec3 receiver_;
    core::environment environment_;
    const core::voxelised_scene_data<cl_float3,
                                     core::surface<core::simulation_bands>>&
            voxelised_;
    size_t num_directions_;

    size_t max_order_;

    raytracer::image_source::tree tree_;
};

////////////////////////////////////////////////////////////////////////////////

class make_image_source final {
public:
    make_image_source(size_t max_order);

    image_source_processor get_processor(
            const core::compute_context& cc,
            const glm::vec3& source,
            const glm::vec3& receiver,
            const core::environment& environment,
            const core::voxelised_scene_data<
                    cl_float3,
                    core::surface<core::simulation_bands>>& voxelised) const;

private:
    size_t max_order_;
};

}  // namespace reflection_processor
}  // namespace raytracer
}  // namespace wayverb
