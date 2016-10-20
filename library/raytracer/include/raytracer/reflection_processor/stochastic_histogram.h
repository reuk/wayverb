#pragma once

#include "raytracer/histogram.h"
#include "raytracer/stochastic/finder.h"
#include "raytracer/stochastic/postprocessing.h"

#include "common/spatial_division/scene_buffers.h"

namespace raytracer {
namespace reflection_processor {

class stochastic_histogram final {
public:
    /// A max_image_source_order of 0 = direct energy from image-source
    /// An order of 1 = direct and one reflection from image-source
    /// i.e. the order == the number of reflections for each image
    stochastic_histogram(const compute_context& cc,
                         const model::parameters& params,
                         float receiver_radius,
                         float sample_rate,
                         size_t max_image_source_order,
                         size_t items);

    template <typename It>
    void process(It b,
                 It e,
                 const scene_buffers& buffers,
                 size_t step,
                 size_t total) {
        const auto output = finder_.process(b, e, buffers);
        const auto to_histogram = [&](auto& in) {
            const auto make_iterator = [&](auto it) {
                return make_histogram_iterator(std::move(it),
                                               params_.speed_of_sound);
            };

            constexpr auto max_time = 60.0;
            incremental_histogram(histogram_,
                                  make_iterator(begin(in)),
                                  make_iterator(end(in)),
                                  sample_rate_,
                                  max_time,
                                  dirac_sum_functor{});
        };

        to_histogram(output.stochastic);
        if (max_image_source_order_ <= step) {
            to_histogram(output.specular);
        }
    }

    stochastic::energy_histogram get_results();

private:
    stochastic::finder finder_;
    model::parameters params_;
    float sample_rate_;
    size_t max_image_source_order_;

    aligned::vector<bands_type> histogram_;
};

class make_stochastic_histogram final {
public:
    make_stochastic_histogram(float receiver_radius,
                              float sample_rate,
                              size_t max_order);

    stochastic_histogram operator()(
            const compute_context& cc,
            const model::parameters& params,
            const voxelised_scene_data<cl_float3, surface<simulation_bands>>&
                    voxelised,
            size_t num_directions) const;

private:
    float receiver_radius_;
    float sample_rate_;
    size_t max_order_;
};

}  // namespace reflection_processor
}  // namespace raytracer
