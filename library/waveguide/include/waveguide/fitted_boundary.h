#pragma once

#include "waveguide/arbitrary_magnitude_filter.h"
#include "waveguide/cl/filter_structs.h"
#include "waveguide/filters.h"
#include "waveguide/stable.h"

#include "common/cosine_interp.h"
#include "common/filter_coefficients.h"
#include "common/surfaces.h"

#include "hrtf/multiband.h"

#include "utilities/aligned/vector.h"
#include "utilities/for_each.h"
#include "utilities/map.h"

namespace waveguide {

namespace detail {
template <size_t... Ix>
constexpr auto to_raw_impedance_coefficients(
        const coefficients<sizeof...(Ix) - 1>& c, std::index_sequence<Ix...>) {
    return coefficients_canonical{{c.a[Ix] + c.b[Ix]...},
                                  {c.a[Ix] - c.b[Ix]...}};
}

template <size_t order>
constexpr auto to_raw_impedance_coefficients(const coefficients<order>& c) {
    return to_raw_impedance_coefficients(c,
                                         std::make_index_sequence<order + 1>{});
}
}  // namespace detail

template <size_t order>
auto to_impedance_coefficients(const coefficients<order>& c) {
    auto ret = detail::to_raw_impedance_coefficients(c);

    if (ret.a[0]) {
        const auto norm = 1.0 / ret.a[0];
        const auto do_normalize = [&](auto& i) {
            for_each([&](auto& i) { i *= norm; }, i);
        };
        do_normalize(ret.b);
        do_normalize(ret.a);
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////

template <size_t... Ix>
constexpr auto make_coefficients_canonical(
        const filter_coefficients<sizeof...(Ix) - 1, sizeof...(Ix) - 1>& coeffs,
        std::index_sequence<Ix...>) {
    return coefficients_canonical{{std::get<Ix>(coeffs.b)...},
                                  {std::get<Ix>(coeffs.a)...}};
}

constexpr auto make_coefficients_canonical(
        const filter_coefficients<coefficients_canonical::order,
                                  coefficients_canonical::order>& coeffs) {
    return make_coefficients_canonical(
            coeffs,
            std::make_index_sequence<coefficients_canonical::order + 1>{});
}

////////////////////////////////////////////////////////////////////////////////

template <size_t Channels>
coefficients_canonical to_flat_coefficients(const surface<Channels>& surface) {
    const auto reflectance =
            absorption_to_pressure_reflectance(surface.absorption.s[0]);
    const coefficients_canonical ret{{reflectance}, {1}};
    return to_impedance_coefficients(ret);
}

template <size_t Channels>
aligned::vector<coefficients_canonical> to_flat_coefficients(
        const aligned::vector<surface<Channels>>& surfaces) {
    return map_to_vector(begin(surfaces), end(surfaces), [](auto i) {
        return to_flat_coefficients(i);
    });
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
auto compute_reflectance_filter_coefficients(T&& absorption,
                                             double sample_rate) {
    const auto band_centres = map([](auto i) { return i * 2; },
                                  hrtf_data::hrtf_band_centres(sample_rate));
    const auto reflectance =
            map([](double i) { return absorption_to_pressure_reflectance(i); },
                absorption);

    constexpr auto lim = 1000;
    for (auto delay = 0; delay != lim; ++delay) {
        const auto reflectance_coeffs = make_coefficients_canonical(
                arbitrary_magnitude_filter<coefficients_canonical::order>(
                        make_frequency_domain_envelope(band_centres,
                                                       reflectance)));

        if (is_stable(reflectance_coeffs.a)) {
            return reflectance_coeffs;
        }
    }

    throw std::runtime_error{"unable to generate stable boundary filter"};
}

}  // namespace waveguide
