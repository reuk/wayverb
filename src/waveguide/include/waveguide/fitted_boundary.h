#pragma once

#include "waveguide/arbitrary_magnitude_filter.h"
#include "waveguide/cl/filter_structs.h"
#include "waveguide/filters.h"
#include "waveguide/stable.h"

#include "core/cosine_interp.h"
#include "core/filter_coefficients.h"
#include "core/surfaces.h"

#include "hrtf/multiband.h"

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
            util::for_each([&](auto& i) { i *= norm; }, i);
        };
        do_normalize(ret.b);
        do_normalize(ret.a);
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////

template <size_t... Ix>
constexpr auto make_coefficients_canonical(
        const core::filter_coefficients<sizeof...(Ix) - 1, sizeof...(Ix) - 1>&
                coeffs,
        std::index_sequence<Ix...>) {
    return coefficients_canonical{{std::get<Ix>(coeffs.b)...},
                                  {std::get<Ix>(coeffs.a)...}};
}

constexpr auto make_coefficients_canonical(
        const core::filter_coefficients<coefficients_canonical::order,
                                        coefficients_canonical::order>&
                coeffs) {
    return make_coefficients_canonical(
            coeffs,
            std::make_index_sequence<coefficients_canonical::order + 1>{});
}

////////////////////////////////////////////////////////////////////////////////

inline auto to_flat_coefficients(double absorption) {
    return to_impedance_coefficients(coefficients_canonical{
            {core::absorption_to_pressure_reflectance(absorption)}, {1}});
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
auto compute_reflectance_filter_coefficients(T&& absorption,
                                             double sample_rate) {
    const auto band_centres =
            util::map([](auto i) { return i * 2; },
                      hrtf_data::hrtf_band_centres(sample_rate));
    const auto reflectance = util::map(
            [](double i) {
                return core::absorption_to_pressure_reflectance(i);
            },
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
