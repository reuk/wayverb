#pragma once

#include "common/aligned/vector.h"
#include "common/sinc.h"
#include "common/specular_absorption.h"

namespace raytracer {

template <typename It, typename T>
auto histogram(It begin,
               It end,
               double speed_of_sound,
               double sample_rate,
               double max_time,
               const T& callback) {
    using value_type = decltype(begin->volume);

    if (begin == end) {
        return aligned::vector<value_type>{};
    }
    const auto max_distance_in_input{
            std::max_element(begin, end, [](auto i, auto j) {
                return i.distance < j.distance;
            })->distance};
    const auto max_t{
            std::min(max_distance_in_input / speed_of_sound, max_time)};
    const size_t output_size = std::round(max_t * sample_rate) + 1;

    aligned::vector<value_type> ret{output_size, value_type{}};
    for (auto i{begin}; i != end; ++i) {
        const auto time{i->distance / speed_of_sound};
        callback(i->volume,
                 time,
                 sample_rate,
                 ret.begin(),
                 ret.end());
    }

    return ret;
}

template <typename T, typename It>
void dirac_sum(T value,
               double time,
               double sample_rate,
               It begin,
               It end) {}

template <typename It>
auto dirac_histogram(It begin,
                     It end,
                     double speed_of_sound,
                     double sample_rate,
                     double max_time) {
    return histogram(
            begin,
            end,
            speed_of_sound,
            sample_rate,
            max_time,
            dirac_sum<volume_type, aligned::vector<volume_type>::iterator>);
}

template <typename T, typename It>
void sinc_sum(T value,
              double time,
              double sample_rate,
              It begin,
              It end) {
    const auto width{400};  //  Impulse width in samples.

    const auto centre_sample{time * sample_rate};
    const size_t ideal_begin = std::floor(centre_sample - width / 2);
    const size_t ideal_end = std::ceil(centre_sample + width / 2);

    const auto begin_samp{std::max(size_t{0}, ideal_begin)};
    const auto end_samp{std::min(static_cast<size_t>(std::distance(begin, end)),
                                 ideal_end)};

    const auto begin_it{begin + begin_samp};
    const auto end_it{begin + end_samp};

    const auto nyquist{sample_rate / 2};

    for (auto i{begin_it}; i != end_it; ++i) {
        const auto this_time{std::distance(begin, i) / sample_rate};
        const auto relative_time{this_time - time};
        const auto angle{2 * M_PI * relative_time};
        const auto envelope{0.5 * (1 + std::cos(angle / width))};
        const auto filt{sinc(angle * nyquist)};
        *i += value * envelope * filt;
    }
}

template <typename T>
class WhatType;

template <typename It>
auto sinc_histogram(It begin,
                    It end,
                    double speed_of_sound,
                    double sample_rate,
                    double max_time) {
    using value_type = decltype(begin->volume);
    return histogram(begin,
                     end,
                     speed_of_sound,
                     sample_rate,
                     max_time,
                     sinc_sum<value_type,
                              typename aligned::vector<value_type>::iterator>);
}

}  // namespace raytracer
