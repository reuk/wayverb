#pragma once

#include "raytracer/simulation_parameters.h"

#include <cmath>

namespace wayverb {
namespace raytracer {

constexpr auto mean_hits_per_interval(const simulation_parameters& params,
                                      double speed_of_sound,
                                      double room_volume) {
    return (params.rays * M_PI * params.receiver_radius *
            params.receiver_radius * speed_of_sound) /
           (room_volume * params.histogram_sample_rate);
}

constexpr auto required_rays(double hits_per_interval,
                             double receiver_radius,
                             double speed_of_sound,
                             double histogram_sample_rate,
                             double room_volume) {
    return (hits_per_interval * room_volume * histogram_sample_rate) /
           (M_PI * receiver_radius * receiver_radius * speed_of_sound);
}

constexpr auto make_simulation_parameters(
            double hits_per_interval,
            double receiver_radius,
            double speed_of_sound,
            double histogram_sample_rate,
            double room_volume,
            size_t maximum_image_source_order) {
    return simulation_parameters{
            static_cast<size_t>(required_rays(hits_per_interval,
                                              receiver_radius,
                                              speed_of_sound,
                                              histogram_sample_rate,
                                              room_volume)),
            maximum_image_source_order,
            receiver_radius,
            histogram_sample_rate};
}

}  // namespace raytracer
}  // namespace wayverb
