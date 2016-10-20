#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/iterative_builder.h"

#include "common/cl/common.h"
#include "common/model/parameters.h"
#include "common/spatial_division/scene_buffers.h"

namespace raytracer {
namespace reflection_processor {

class visual final {
public:
    explicit visual(size_t items);

    template <typename It>
    void process(It b,
                 It e,
                 const scene_buffers& buffers,
                 size_t step,
                 size_t total) {
        builder_.push(b, b + builder_.get_num_items());
    }

    aligned::vector<aligned::vector<reflection>> get_results();

private:
    iterative_builder<reflection> builder_;
};

class make_visual final {
public:
    explicit make_visual(size_t items);

    visual operator()(const compute_context& cc,
                      const model::parameters& params,
                      const voxelised_scene_data<cl_float3, surface>& voxelised,
                      size_t num_directions) const;

private:
    size_t items_;
};

}  // namespace reflection_processor
}  // namespace raytracer
