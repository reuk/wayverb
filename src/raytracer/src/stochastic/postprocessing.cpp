#include "raytracer/stochastic/postprocessing.h"

#include "core/cl/iterator.h"
#include "core/mixdown.h"
#include "core/pressure_intensity.h"

#include "utilities/for_each.h"
#include "utilities/map.h"

#include <iostream>

namespace wayverb {
namespace raytracer {
namespace stochastic {

double constant_mean_event_occurrence(double speed_of_sound,
                                      double room_volume) {
    return 4 * M_PI * std::pow(speed_of_sound, 3.0) / room_volume;
}

double mean_event_occurrence(double constant, double t) {
    return std::min(constant * std::pow(t, 2.0), 10000.0);
}

double t0(double constant) {
    return std::pow(2.0 * std::log(2.0) / constant, 1.0 / 3.0);
}

dirac_sequence generate_dirac_sequence(double speed_of_sound,
                                       double room_volume,
                                       double sample_rate,
                                       double max_time) {
    const auto constant_mean_occurrence =
            constant_mean_event_occurrence(speed_of_sound, room_volume);

    std::default_random_engine engine{std::random_device{}()};

    util::aligned::vector<float> ret(std::ceil(max_time * sample_rate), 0);
    for (auto t = t0(constant_mean_occurrence); t < max_time;
         t += interval_size(
                 engine, mean_event_occurrence(constant_mean_occurrence, t))) {
        const auto sample_index = t * sample_rate;
        const size_t twice = 2 * sample_index;
        const bool negative = (twice % 2) != 0;
        ret[sample_index] = negative ? -1 : 1;
    }
    return {ret, sample_rate};
}

void sum_histograms(energy_histogram& a, const energy_histogram& b) {
    sum_vectors(a.histogram, b.histogram);
    a.sample_rate = b.sample_rate;
}

util::aligned::vector<core::bands_type> weight_sequence(
        const energy_histogram& histogram,
        const dirac_sequence& sequence,
        double acoustic_impedance) {
    auto ret = util::map_to_vector(
            begin(sequence.sequence), end(sequence.sequence), [](auto i) {
                return core::make_bands_type(i);
            });

    const auto convert_index = [&](auto ind) -> size_t {
        return ind * sequence.sample_rate / histogram.sample_rate;
    };

    const auto ideal_sequence_length =
            convert_index(histogram.histogram.size());
    if (ideal_sequence_length < ret.size()) {
        ret.resize(ideal_sequence_length);
    }

    for (auto i = 0ul, e = histogram.histogram.size(); i != e; ++i) {
        const auto get_sequence_index = [&](auto ind) {
            return std::min(convert_index(ind), ret.size());
        };

        const auto beg = get_sequence_index(i);
        const auto end = get_sequence_index(i + 1);

        const auto squared_summed = frequency_domain::square_sum(
                begin(sequence.sequence) + beg, begin(sequence.sequence) + end);
        const auto scale_factor =
                squared_summed != 0.0f
                        ? core::intensity_to_pressure(
                                  histogram.histogram[i] / squared_summed,
                                  acoustic_impedance)
                        : cl_double8{};

        std::for_each(begin(ret) + beg, begin(ret) + end, [&](auto& i) {
            i *= scale_factor;
        });
    }

    return ret;
}

util::aligned::vector<float> postprocessing(const energy_histogram& histogram,
                                            const dirac_sequence& sequence,
                                            double acoustic_impedance) {
    auto weighted = weight_sequence(histogram, sequence, acoustic_impedance);
    return core::multiband_filter_and_mixdown(
            begin(weighted),
            end(weighted),
            sequence.sample_rate,
            [](auto it, auto index) {
                return core::make_cl_type_iterator(std::move(it), index);
            });
}

}  // namespace stochastic
}  // namespace raytracer
}  // namespace wayverb
