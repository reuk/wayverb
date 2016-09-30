#pragma once

#include "common/aligned/vector.h"
#include "common/sinc.h"
#include "common/specular_absorption.h"

namespace detail {
struct histogram_mapper final {
    double speed_of_sound;

    template <typename T>
    auto operator()(const T& i) const {
        struct {
            decltype(i.volume) volume;
            double time;
        } ret{i.volume, i.distance / speed_of_sound};
        return ret;
    }
};
}  // namespace detail

namespace raytracer {

template <typename T>
auto make_histogram_iterator(T t, double speed_of_sound) {
    return make_mapping_iterator_adapter(
            std::move(t), ::detail::histogram_mapper{speed_of_sound});
}

template <typename T>
auto time(const T& t) {
    return t.time;
}

template <typename T>
auto volume(const T& t) {
    return t.volume;
}

template <typename It, typename T>
auto histogram(It begin,
               It end,
               double sample_rate,
               double max_time,
               const T& callback) {
    using value_type = decltype(volume(*begin));

    if (begin == end) {
        return aligned::vector<value_type>{};
    }
    const auto max_time_in_input{time(*std::max_element(
            begin, end, [](auto i, auto j) { return time(i) < time(j); }))};
    const auto max_t{std::min(max_time_in_input, max_time)};
    const size_t output_size = std::floor(max_t * sample_rate) + 1;

    aligned::vector<value_type> ret(output_size, value_type{});
    for (auto i{begin}; i != end; ++i) {
        const auto item_time{time(*i)};
        const auto centre_sample{item_time * sample_rate};
        if (0 <= centre_sample && centre_sample < ret.size()) {
            callback(volume(*i), item_time, sample_rate, ret.begin(), ret.end());
        }
    }

    return ret;
}

template <typename T, typename It>
void dirac_sum(T value, double item_time, double sample_rate, It begin, It end) {
    const auto centre_sample{item_time * sample_rate};
    *(begin + centre_sample) += value;
}

template <typename It>
auto dirac_histogram(It begin, It end, double sample_rate, double max_time) {
    return histogram(begin,
                     end,
                     sample_rate,
                     max_time,
                     [](auto value, auto time, auto sr, auto begin, auto end) {
                         dirac_sum(value, time, sr, begin, end);
                     });
}

/// See fu2015 2.2.2 'Discrete form of the impulse response'
template <typename T, typename It>
void sinc_sum(T value, double item_time, double sample_rate, It begin, It end) {
    constexpr auto width{400};  //  Impulse width in samples.

    const auto centre_sample{item_time * sample_rate};

    const ptrdiff_t ideal_begin = std::floor(centre_sample - width / 2);
    const ptrdiff_t ideal_end = std::ceil(centre_sample + width / 2);

    const auto begin_samp{std::max(static_cast<ptrdiff_t>(0), ideal_begin)};
    const auto end_samp{std::min(std::distance(begin, end),
                                 ideal_end)};

    const auto begin_it{begin + begin_samp};
    const auto end_it{begin + end_samp};

    for (auto i{begin_it}; i != end_it; ++i) {
        const auto this_time{std::distance(begin, i) / sample_rate};
        const auto relative_time{this_time - item_time};
        const auto angle{2 * M_PI * relative_time};
        const auto envelope{0.5 * (1 + std::cos(angle / width))};
        const auto filt{sinc(relative_time * sample_rate)};
        *i += value * envelope * filt;
    }
}

template <typename It>
auto sinc_histogram(It begin, It end, double sample_rate, double max_time) {
    return histogram(begin,
                     end,
                     sample_rate,
                     max_time,
                     [](auto value, auto time, auto sr, auto begin, auto end) {
                         sinc_sum(value, time, sr, begin, end);
                     });
}

}  // namespace raytracer
