#include "raytracer/reflection_processor/stochastic_histogram.h"

namespace raytracer {
namespace reflection_processor {

stochastic_histogram::stochastic_histogram(const compute_context& cc,
                                           const model::parameters& params,
                                           float receiver_radius,
                                           float sample_rate,
                                           size_t max_image_source_order,
                                           size_t items)
        : finder_{cc, params, receiver_radius, items}
        , params_{params}
        , sample_rate_{sample_rate}
        , max_image_source_order_{max_image_source_order} {}

stochastic::energy_histogram stochastic_histogram::get_results() {
    return {std::move(histogram_), sample_rate_};
}

make_stochastic_histogram::make_stochastic_histogram(float receiver_radius,
                                                     float sample_rate,
                                                     size_t max_order)
        : receiver_radius_{receiver_radius}
        , sample_rate_{sample_rate}
        , max_order_{max_order} {}

stochastic_histogram make_stochastic_histogram::operator()(
        const compute_context& cc,
        const model::parameters& params,
        const voxelised_scene_data<cl_float3, surface<simulation_bands>>&
                voxelised,
        size_t num_directions) const {
    return {cc,
            params,
            receiver_radius_,
            sample_rate_,
            max_order_,
            num_directions};
}

}  // namespace reflection_processor
}  // namespace raytracer
