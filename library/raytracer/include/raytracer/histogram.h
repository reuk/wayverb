#pragma once

#include "common/sinc.h"

#include "utilities/aligned/vector.h"
#include "utilities/mapping_iterator_adapter.h"

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
void incremental_histogram(Ret& ret,
                           It b,
                           It e,
                           double sample_rate,
                           double max_time,
                           const T& callback) {
    const auto make_time_iterator = [](auto it) {
        return make_mapping_iterator_adapter(std::move(it), time_functor{});
    };
    const auto max_time_in_input = std::min(
            *std::max_element(make_time_iterator(b), make_time_iterator(e)),
            max_time);

    resize_if_necessary(ret, std::floor(max_time_in_input * sample_rate) + 1);

    for (; b != e; ++b) {
        const auto item_time = time(*b);
        if (item_time < max_time_in_input) {
            callback(*b, item_time, sample_rate, ret);
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

template <typename T, typename U, typename Alloc>
void dirac_sum(const T& item,
               double item_time,
               double sample_rate,
               std::vector<U, Alloc>& ret) {
    ret[item_time * sample_rate] += volume(item);
}

struct dirac_sum_functor final {
    template <typename T, typename Ret>
    void operator()(const T& item,
                    double item_time,
                    double sample_rate,
                    Ret& ret) const {
        dirac_sum(item, item_time, sample_rate, ret);
    }
};

/// See fu2015 2.2.2 'Discrete form of the impulse response'
struct sinc_sum_functor final {
    template <typename T, typename Ret>
    void operator()(const T& item,
                    double item_time,
                    double sample_rate,
                    Ret& ret) const {
        constexpr auto width = 400;  //  Impulse width in samples.

        const auto centre_sample = item_time * sample_rate;

        const ptrdiff_t ideal_begin = std::floor(centre_sample - width / 2);
        const ptrdiff_t ideal_end = std::ceil(centre_sample + width / 2);
        ret.resize(std::max(ret.size(), static_cast<size_t>(ideal_end)));

        const auto begin_samp =
                std::max(static_cast<ptrdiff_t>(0), ideal_begin);
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
            *i += volume(item) * envelope * filt;
        }
    }
};

//----------------------------------------------------------------------------//

/// These functions are for volume/distance pairs rather than volume/time.

template <typename T>
auto make_histogram_iterator(T t, double speed_of_sound) {
    if (speed_of_sound < 300 || 400 <= speed_of_sound) {
        throw std::runtime_error{"speed_of_sound outside expected range"};
    }
    return make_mapping_iterator_adapter(std::move(t),
                                         histogram_mapper{speed_of_sound});
}

}  // namespace raytracer
