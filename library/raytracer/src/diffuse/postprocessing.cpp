#include "raytracer/diffuse/postprocessing.h"

#include "common/cl/iterator.h"
#include "common/mixdown.h"

#include "utilities/map.h"

#include <iostream>

namespace raytracer {

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

aligned::vector<float> generate_dirac_sequence(double speed_of_sound,
                                               double room_volume,
                                               double sample_rate,
                                               double max_time) {
    const auto constant_mean_occurrence{
            constant_mean_event_occurrence(speed_of_sound, room_volume)};

    std::default_random_engine engine{std::random_device{}()};

    aligned::vector<float> ret(std::ceil(max_time * sample_rate), 0);
    for (auto t{t0(constant_mean_occurrence)}; t < max_time;
         t += interval_size(
                 engine, mean_event_occurrence(constant_mean_occurrence, t))) {
        const auto sample_index{t * sample_rate};
        const size_t twice = 2 * sample_index;
        const bool negative = twice % 2;
        ret[sample_index] = negative ? -1 : 1;
    }
    return ret;
}

template <typename T, size_t... Ix>
constexpr auto array_to_volume_type(const std::array<T, 8>& t,
                                    std::index_sequence<Ix...>) {
    return volume_type{{static_cast<float>(t[Ix])...}};
}

template <typename T>
constexpr auto array_to_volume_type(const std::array<T, 8>& t) {
    return array_to_volume_type(t, std::make_index_sequence<8>{});
}

dirac_sequence prepare_dirac_sequence(double speed_of_sound,
                                      double room_volume,
                                      double sample_rate,
                                      double max_time) {
    const auto single{generate_dirac_sequence(
            speed_of_sound, room_volume, sample_rate, max_time)};
    auto multi{map_to_vector(begin(single), end(single), [](auto i) {
        return make_volume_type(i);
    })};

    const auto params{hrtf_data::hrtf_band_params(sample_rate)};

    frequency_domain::multiband_filter(
            begin(multi), end(multi), params, [](auto it, auto index) {
                return make_cl_type_iterator(std::move(it), index);
            });

    return {multi,
            array_to_volume_type(frequency_domain::bandwidths(params)),
            sample_rate};
}

std::ostream& operator<<(std::ostream& o, const volume_type& v) {
    o << "[";
    for (const auto & i : v.s) {
        o << i << ", ";
    }
    return o << "]";
}

void weight_sequence(aligned::vector<volume_type>& sequence,
                     const volume_type& bandwidths,
                     double sequence_sample_rate,
                     const aligned::vector<volume_type>& histogram,
                     double histogram_sample_rate) {
    //  If the dirac sequence is longer than the histogram data we have, we
    //  shorten it.
    //  This avoids weird noise stuff at the end of the processed sequence.
    const auto convert_index{[&](auto ind) -> size_t {
        return ind * sequence_sample_rate / histogram_sample_rate;
    }};
    const auto ideal_sequence_length{convert_index(histogram.size())};
    if (ideal_sequence_length < sequence.size()) {
        sequence.resize(ideal_sequence_length);
    }

    for (auto i{0ul}, e{histogram.size()}; i != e; ++i) {
        const auto get_sequence_index{[&](auto ind) {
            return sequence.begin() +
                   std::min(convert_index(ind), sequence.size());
        }};
        const auto beg{get_sequence_index(i)};
        const auto end{get_sequence_index(i + 1)};
        const auto sequence_energy{frequency_domain::square_sum(beg, end)};

        const auto scale_factor{
                sqrt(2 * histogram[i] * bandwidths / sequence_energy)};
        for_each(beg, end, [&](auto& i) { i *= scale_factor; });
    }
}

aligned::vector<float> mono_diffuse_postprocessing(
        const energy_histogram& diff, const dirac_sequence& sequence) {
    auto copy{sequence.sequence};
    weight_sequence(copy,
                    sequence.bandwidths,
                    sequence.sample_rate,
                    diff.full_histogram,
                    diff.sample_rate);
    return mixdown(begin(copy), end(copy));
}

}  // namespace raytracer
