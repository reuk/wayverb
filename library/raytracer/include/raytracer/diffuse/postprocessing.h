#pragma once

#include "common/cl/scene_structs.h"
#include "common/model/parameters.h"

#include "hrtf/multiband.h"

#include "utilities/aligned/vector.h"

#include <array>
#include <cmath>
#include <random>

namespace raytracer {

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

aligned::vector<float> generate_dirac_sequence(double speed_of_sound,
                                               double room_volume,
                                               double sample_rate,
                                               double max_time);

void weight_sequence(aligned::vector<volume_type>& sequence,
                     const volume_type& bandwidths,
                     double sequence_sample_rate,
                     const aligned::vector<volume_type>& histogram,
                     double histogram_sample_rate);

struct dirac_sequence final {
    aligned::vector<volume_type> sequence;
    volume_type bandwidths;
    double sample_rate;
};

dirac_sequence prepare_dirac_sequence(double speed_of_sound,
                                      double room_volume,
                                      double sample_rate,
                                      double max_time);

struct diffuse_results final {
    aligned::vector<volume_type> full_histogram;
    double sample_rate;
};

aligned::vector<float> mono_diffuse_postprocessing(
        const diffuse_results& diff, const dirac_sequence& sequence);

}  // namespace raytracer
