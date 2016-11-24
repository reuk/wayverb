#pragma once

#include "core/sinc.h"

#include "utilities/aligned/vector.h"
#include "utilities/mapping_iterator_adapter.h"

#include <iostream>

namespace wayverb {
namespace raytracer {

template <typename T>
constexpr auto time(const T& t) {
    return t.time;
}

template <typename T>
constexpr auto volume(const T& t) {
    return t.volume;
}

template <typename T>
struct item_with_time final {
    T item;
    double time;
};

template <typename T>
constexpr auto make_item_with_time(T item, double time) {
    return item_with_time<T>{std::move(item), time};
}

template <typename T>
constexpr auto volume(const item_with_time<T>& t) {
    return volume(t.item);
}

struct histogram_mapper final {
    double speed_of_sound;

    template <typename T>
    auto operator()(const T& i) const {
        return make_item_with_time(i, i.distance / speed_of_sound);
    }
};

struct time_functor final {
    template <typename T>
    constexpr auto operator()(const T& t) const {
        return time(t);
    }
};

template <typename T, typename Alloc>
void resize_if_necessary(std::vector<T, Alloc>& t, size_t new_size) {
    if (t.size() < new_size) {
        t.resize(new_size);
    }
}

template <typename Ret, typename It, typename T>
void incremental_histogram(
        Ret& ret, It b, It e, double sample_rate, const T& callback) {
    if (std::distance(b, e)) {
        const auto make_time_iterator = [](auto it) {
            return util::make_mapping_iterator_adapter(std::move(it),
                                                       time_functor{});
        };
        const auto max_time_in_input =
                *std::max_element(make_time_iterator(b), make_time_iterator(e));

        resize_if_necessary(ret,
                            std::floor(max_time_in_input * sample_rate) + 1);

        for (; b != e; ++b) {
            callback(*b, sample_rate, ret);
        }
    }
}

template <typename It, typename T>
auto histogram(It b, It e, double sample_rate, const T& callback) {
    using value_type = decltype(volume(*b));
    util::aligned::vector<value_type> ret{};
    incremental_histogram(ret, b, e, sample_rate, callback);
    return ret;
}

template <typename T, typename U, typename Alloc>
void dirac_sum(const T& item, double sample_rate, std::vector<U, Alloc>& ret) {
    ret[time(item) * sample_rate] += volume(item);
}

struct dirac_sum_functor final {
    template <typename T, typename Ret>
    void operator()(const T& item, double sample_rate, Ret& ret) const {
        dirac_sum(item, sample_rate, ret);
    }
};

/// See fu2015 2.2.2 'Discrete form of the impulse response'
struct sinc_sum_functor final {
    template <typename T, typename Ret>
    void operator()(const T& item, double sample_rate, Ret& ret) const {
        constexpr auto width = 400;  //  Impulse width in samples.

        const auto item_time = time(item);
        const auto centre_sample = item_time * sample_rate;

        const ptrdiff_t ideal_begin = std::floor(centre_sample - width / 2);
        const ptrdiff_t ideal_end = std::ceil(centre_sample + width / 2);
        ret.resize(std::max(ret.size(), static_cast<size_t>(ideal_end)));

        const auto begin_samp =
                std::max(static_cast<ptrdiff_t>(0), ideal_begin);
        const auto end_samp =
                std::min(static_cast<ptrdiff_t>(ret.size()), ideal_end);

        for (auto it = ret.begin() + begin_samp,
                  end_it = ret.begin() + end_samp;
             it != end_it;
             ++it) {
            const auto this_sample = std::distance(ret.begin(), it);
            const auto relative_sample = this_sample - centre_sample;
            const auto envelope =
                    0.5 * (1 + std::cos(2 * M_PI * relative_sample / width));
            const auto filt = core::sinc(relative_sample);
            *it += volume(item) * envelope * filt;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////

/// These functions are for volume/distance pairs rather than volume/time.

template <typename T>
auto make_histogram_iterator(T t, double speed_of_sound) {
    if (speed_of_sound < 300 || 400 <= speed_of_sound) {
        throw std::runtime_error{"Speed_of_sound outside expected range."};
    }
    return util::make_mapping_iterator_adapter(
            std::move(t), histogram_mapper{speed_of_sound});
}

}  // namespace raytracer
}  // namespace wayverb
