#pragma once

#include "waveguide/cl/filter_structs.h"

#include "utilities/decibels.h"

#include <cmath>
#include <functional>

namespace waveguide {

/// absolute gain (0 = silent, 1 = unity, 2 = double amplitude etc.)
/// normalized centre frequency (0 = dc, 0.5 = nyquist, 1 = sr)
/// Q (y'know, Q)
struct filter_descriptor final {
    double gain{0};
    double centre{0};
    double Q{0};
};

template <typename Callback, size_t... Ix>
inline biquad_coefficients_array get_biquads_array(
        const Callback& callback,
        const std::array<filter_descriptor, biquad_sections>& n,
        std::index_sequence<Ix...>) {
    return biquad_coefficients_array{{callback(std::get<Ix>(n))...}};
}

template <typename Callback>
inline biquad_coefficients_array get_biquads_array(
        const Callback& callback,
        const std::array<filter_descriptor, biquad_sections>& n) {
    return get_biquads_array(
            callback, n, std::make_index_sequence<biquad_sections>{});
}

coefficients_biquad get_peak_coefficients(const filter_descriptor& n);

biquad_coefficients_array get_peak_biquads_array(
        const std::array<filter_descriptor, biquad_sections>& n);

template <size_t A, size_t B>
constexpr coefficients<A + B> convolve(const coefficients<A>& a,
                                       const coefficients<B>& b) {
    auto ret = coefficients<A + B>{};
    for (auto i = 0; i != A + 1; ++i) {
        for (auto j = 0; j != B + 1; ++j) {
            ret.b[i + j] += a.b[i] * b.b[j];
            ret.a[i + j] += a.a[i] * b.a[j];
        }
    }
    return ret;
}

coefficients_canonical convolve(const biquad_coefficients_array& a);

}  // namespace waveguide
