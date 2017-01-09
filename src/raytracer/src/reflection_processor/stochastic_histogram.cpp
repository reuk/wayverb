#include "raytracer/reflection_processor/stochastic_histogram.h"

namespace wayverb {
namespace raytracer {
namespace reflection_processor {

make_stochastic_histogram::make_stochastic_histogram(
        size_t total_rays,
        size_t max_image_source_order,
        float receiver_radius,
        float histogram_sample_rate)
        : total_rays_{total_rays}
        , max_image_source_order_{max_image_source_order}
        , receiver_radius_{receiver_radius}
        , histogram_sample_rate_{histogram_sample_rate} {}

stochastic_processor<stochastic::energy_histogram>
make_stochastic_histogram::get_processor(
        const core::compute_context& cc,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
        /*voxelised*/) const {
    return {cc,
            source,
            receiver,
            environment,
            total_rays_,
            max_image_source_order_,
            receiver_radius_,
            histogram_sample_rate_};
}

////////////////////////////////////////////////////////////////////////////////

make_directional_histogram::make_directional_histogram(
        size_t total_rays,
        size_t max_image_source_order,
        float receiver_radius,
        float histogram_sample_rate)
        : total_rays_{total_rays}
        , max_image_source_order_{max_image_source_order}
        , receiver_radius_{receiver_radius}
        , histogram_sample_rate_{histogram_sample_rate} {}

stochastic_processor<stochastic::directional_energy_histogram<20, 9>>
make_directional_histogram::get_processor(
        const core::compute_context& cc,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
        /*voxelised*/) const {
    return {cc,
            source,
            receiver,
            environment,
            total_rays_,
            max_image_source_order_,
            receiver_radius_,
            histogram_sample_rate_};
}

}  // namespace reflection_processor
}  // namespace raytracer
}  // namespace wayverb
