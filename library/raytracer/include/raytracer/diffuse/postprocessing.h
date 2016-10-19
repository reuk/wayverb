#pragma once

#include "common/cl/scene_structs.h"
#include "common/model/parameters.h"

#include "hrtf/multiband.h"

#include "utilities/aligned/vector.h"

#include <array>
#include <cmath>
#include <random>

namespace raytracer {

template <typename T, size_t... Ix>
constexpr auto array_to_volume_type(const std::array<T, 8>& t,
                                    std::index_sequence<Ix...>) {
    return volume_type{{static_cast<float>(t[Ix])...}};
}

template <typename T>
constexpr auto array_to_volume_type(const std::array<T, 8>& t) {
    return array_to_volume_type(t, std::make_index_sequence<8>{});
}

/// See schroder2011 5.3.4. , p.70

double constant_mean_event_occurrence(double speed_of_sound,
                                      double room_volume);
double mean_event_occurrence(double constant, double t);

template <typename Engine>
auto interval_size(Engine& engine, double mean_occurrence) {
    //  Parameters are this way round to preserve the half-open range in the
    //  correct direction.
    std::uniform_real_distribution<double> distribution{1.0, 0.0};
    return 1.0 / mean_occurrence * std::log(1.0 / distribution(engine));
}

double t0(double constant);

struct dirac_sequence final {
    aligned::vector<float> sequence;
    double sample_rate;
};

dirac_sequence generate_dirac_sequence(double speed_of_sound,
                                       double room_volume,
                                       double sample_rate,
                                       double max_time);

struct energy_histogram final {
    aligned::vector<volume_type> full_histogram;
    double sample_rate;
};

aligned::vector<float> mono_diffuse_postprocessing(
        const energy_histogram& histogram,
        const dirac_sequence& sequence,
        double acoustic_impedance);

}  // namespace raytracer
