#include "raytracer/reflection_processor/stochastic_histogram.h"

namespace raytracer {
namespace reflection_processor {

make_stochastic_histogram::make_stochastic_histogram(float receiver_radius,
                                                     float sample_rate,
                                                     size_t max_order)
        : receiver_radius_{receiver_radius}
        , sample_rate_{sample_rate}
        , max_order_{max_order} {}

stochastic_histogram<stochastic::energy_histogram> make_stochastic_histogram::
operator()(const compute_context& cc,
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

////////////////////////////////////////////////////////////////////////////////

make_directional_histogram::make_directional_histogram(float receiver_radius,
                                                       float sample_rate,
                                                       size_t max_order)
        : receiver_radius_{receiver_radius}
        , sample_rate_{sample_rate}
        , max_order_{max_order} {}

stochastic_histogram<stochastic::directional_energy_histogram<20, 9>>
make_directional_histogram::operator()(
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
