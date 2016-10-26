#pragma once

#include "common/attenuator/null.h"
#include "common/cl/iterator.h"
#include "common/cl/scene_structs.h"
#include "common/mixdown.h"
#include "common/model/parameters.h"
#include "common/vector_look_up_table.h"

#include "hrtf/multiband.h"

#include "utilities/aligned/vector.h"

#include <array>
#include <cmath>
#include <random>

namespace raytracer {
namespace stochastic {

template <typename T, size_t... Ix>
constexpr auto array_to_bands_type(const std::array<T, 8>& t,
                                   std::index_sequence<Ix...>) {
    return bands_type{{static_cast<float>(t[Ix])...}};
}

template <typename T>
constexpr auto array_to_bands_type(const std::array<T, 8>& t) {
    return array_to_bands_type(t, std::make_index_sequence<8>{});
}

/// See schroder2011 5.3.4., p.70

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
    double sample_rate;
    aligned::vector<bands_type> histogram;
};

template <size_t Az, size_t El>
struct directional_energy_histogram final {
    double sample_rate;
    vector_look_up_table<aligned::vector<bands_type>, Az, El> histogram;
};

template <size_t Az, size_t El>
auto sum_directional_histogram(
        const directional_energy_histogram<Az, El>& histogram) {
    aligned::vector<bands_type> ret;
    for (auto azimuth_index = 0ul; azimuth_index != Az; ++azimuth_index) {
        for (auto elevation_index = 0ul; elevation_index != El;
             ++elevation_index) {
            const auto& segment =
                    histogram.histogram.table[azimuth_index][elevation_index];
            ret.resize(std::max(ret.size(), segment.size()));
            for (auto i = 0ul, end = segment.size(); i != end; ++i) {
                ret[i] += segment[i];
            }
        }
    }

    return energy_histogram{histogram.sample_rate, ret};
}

template <typename Method>
auto compute_summed_histogram(const energy_histogram& histogram,
                              const Method&) {
    return histogram;
}

//  Special case for the null attenuator.
template <size_t Az, size_t El>
auto compute_summed_histogram(
        const directional_energy_histogram<Az, El>& histogram,
        const attenuator::null& method) {
    return sum_directional_histogram(histogram);
}

template <size_t Az, size_t El, typename Method>
auto compute_summed_histogram(
        const directional_energy_histogram<Az, El>& histogram,
        const Method& method) {
    using hist = std::decay_t<decltype(histogram.histogram)>;

    aligned::vector<bands_type> ret;

    for (auto azimuth_index = 0ul; azimuth_index != Az; ++azimuth_index) {
        for (auto elevation_index = 0ul; elevation_index != El;
             ++elevation_index) {
            //  This is the direction that the histogram segment is pointing,
            //  in world space.
            const auto pointing = hist::pointing(
                    typename hist::index_pair{azimuth_index, elevation_index});

            //  This is the attenuation of the receiver in that direction.
            //  We're dealing in energies, so we square the directional
            //  response.
            const auto att = attenuation(method, pointing);
            const auto factor = att * att;

            const auto& segment =
                    histogram.histogram.table[azimuth_index][elevation_index];

            //  Ensure that the return vector is large enough.
            ret.resize(std::max(ret.size(), segment.size()));

            //  For each histogram bin in this segment, attenuate it
            //  appropriately and add it to the return vector.
            for (auto i = 0ul, end = segment.size(); i != end; ++i) {
                ret[i] += segment[i] * factor;
            }
        }
    }

    return energy_histogram{histogram.sample_rate, ret};
}

aligned::vector<bands_type> weight_sequence(const energy_histogram& histogram,
                                            const dirac_sequence& sequence,
                                            double acoustic_impedance);

aligned::vector<float> postprocessing(const energy_histogram& histogram,
                                      const dirac_sequence& sequence,
                                      double acoustic_impedance);

template <size_t Az, size_t El, typename Method>
aligned::vector<float> postprocessing(
        const directional_energy_histogram<Az, El>& histogram,
        const Method& method,
        const dirac_sequence& sequence,
        double acoustic_impedance) {
    const auto summed = compute_summed_histogram(histogram, method);
    return postprocessing(summed, sequence, acoustic_impedance);
}

}  // namespace stochastic
}  // namespace raytracer
