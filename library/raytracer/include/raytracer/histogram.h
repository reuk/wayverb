#pragma once

#include "common/sinc.h"

#include "utilities/aligned/vector.h"
#include "utilities/mapping_iterator_adapter.h"

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
constexpr auto time(const T& t) {
    return t.time;
}

template <typename T>
constexpr auto volume(const T& t) {
    return t.volume;
}

struct time_functor final {
    template <typename T>
    constexpr auto operator()(const T& t) const {
        return time(t);
    }
};

template <typename V, typename It, typename T>
void incremental_histogram(aligned::vector<V>& ret,
                           It b,
                           It e,
                           double sample_rate,
                           double max_time,
                           const T& callback) {
    const auto make_time_iterator = [](auto it) {
        return make_mapping_iterator_adapter(std::move(it), time_functor{});
    };
    const auto max_time_in_input =
            *std::max_element(make_time_iterator(b), make_time_iterator(e));
    const auto max_t = std::min(max_time_in_input, max_time);
    const size_t output_size = std::floor(max_t * sample_rate) + 1;
    if (ret.size() < output_size) {
        ret.resize(output_size, V());
    }

    for (; b != e; ++b) {
        const auto item_time = time(*b);
        if (item_time < max_time) {
            callback(volume(*b), item_time, sample_rate, ret);
        }
    }    
}

template <typename It, typename T>
auto histogram(
        It b, It e, double sample_rate, double max_time, const T& callback) {
    using value_type = decltype(volume(*b));
    aligned::vector<value_type> ret{};
    incremental_histogram(ret, b, e, sample_rate, max_time, callback);
    return ret;
}

template <typename T>
void dirac_sum(T value,
               double item_time,
               double sample_rate,
               aligned::vector<T>& ret) {
    ret[item_time * sample_rate] += value;
}

struct dirac_sum_functor final {
    template <typename T>
    void operator()(T value,
                    double item_time,
                    double sample_rate,
                    aligned::vector<T>& ret) const {
        dirac_sum(value, item_time, sample_rate, ret);
    }
};

/// See fu2015 2.2.2 'Discrete form of the impulse response'
template <typename T>
void sinc_sum(T value,
              double item_time,
              double sample_rate,
              aligned::vector<T>& ret) {
    constexpr auto width = 400;  //  Impulse width in samples.

    const auto centre_sample = item_time * sample_rate;

    const ptrdiff_t ideal_begin = std::floor(centre_sample - width / 2);
    const ptrdiff_t ideal_end = std::ceil(centre_sample + width / 2);
    ret.resize(std::max(ret.size(), static_cast<size_t>(ideal_end)));

    const auto begin_samp = std::max(static_cast<ptrdiff_t>(0), ideal_begin);
    const auto end_samp =
            std::min(static_cast<ptrdiff_t>(ret.size()), ideal_end);

    const auto begin_it = ret.begin() + begin_samp;
    const auto end_it = ret.begin() + end_samp;

    for (auto i = begin_it; i != end_it; ++i) {
        const auto this_time = std::distance(ret.begin(), i) / sample_rate;
        const auto relative_time = this_time - item_time;
        const auto angle = 2 * M_PI * relative_time;
        const auto envelope = 0.5 * (1 + std::cos(angle / width));
        const auto filt = sinc(relative_time * sample_rate);
        *i += value * envelope * filt;
    }
}

struct sinc_sum_functor final {
    template <typename T>
    void operator()(T value,
                    double item_time,
                    double sample_rate,
                    aligned::vector<T>& ret) const {
        sinc_sum(value, item_time, sample_rate, ret);
    }
};

//----------------------------------------------------------------------------//

/// These functions are for volume/distance pairs rather than volume/time.

template <typename T>
auto make_histogram_iterator(T t, double speed_of_sound) {
    if (speed_of_sound < 300 || 400 <= speed_of_sound) {
        throw std::runtime_error{"speed_of_sound outside expected range"};
    }
    return make_mapping_iterator_adapter(
            std::move(t), ::detail::histogram_mapper{speed_of_sound});
}

}  // namespace raytracer
