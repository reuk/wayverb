#pragma once

#include "raytracer/image_source/reflection_path_builder.h"

#include "common/cl/common.h"
#include "common/model/parameters.h"
#include "common/spatial_division/scene_buffers.h"

namespace raytracer {
namespace reflection_processor {

class image_source final {
public:
    image_source(
            size_t max_order,
            bool flip_phase,
            const model::parameters& params,
            const voxelised_scene_data<cl_float3, surface<simulation_bands>>&
                    voxelised,
            size_t items);

    template <typename It>
    void process(It b,
                 It e,
                 const scene_buffers& buffers,
                 size_t step,
                 size_t total) {
        if (step < max_image_source_order_) {
            builder_.push(b, e);
        }
    }

    aligned::vector<impulse<8>> get_results();

private:
    model::parameters params_;
    const voxelised_scene_data<cl_float3, surface<simulation_bands>>&
            voxelised_;
    size_t max_image_source_order_;
    bool flip_phase_;

    raytracer::image_source::reflection_path_builder builder_;
};

class make_image_source final {
public:
    make_image_source(size_t max_order, bool flip_phase);

    image_source operator()(
            const compute_context& cc,
            const model::parameters& params,
            const voxelised_scene_data<cl_float3, surface<simulation_bands>>&
                    voxelised,
            size_t num_directions) const;

private:
    size_t max_order_;
    bool flip_phase_;
};

}  // namespace reflection_processor
}  // namespace raytracer
