#pragma once

#include "waveguide/cl/filter_structs.h"
#include "waveguide/eqnerror.h"
#include "waveguide/filters.h"
#include "waveguide/stable.h"

#include "common/cosine_interp.h"
#include "common/surfaces.h"

#include "hrtf/multiband.h"

#include "utilities/aligned/vector.h"
#include "utilities/for_each.h"

namespace waveguide {

struct point final {
    double x;
    double y;
};

template <size_t... Ix>
constexpr auto zip_to_point_array(const std::array<double, sizeof...(Ix)>& a,
                                  const std::array<double, sizeof...(Ix)>& b,
                                  std::index_sequence<Ix...>) {
    return std::array<point, sizeof...(Ix)>{
            {{std::get<Ix>(a), std::get<Ix>(b)}...}};
}

template <size_t N>
constexpr auto zip_to_point_array(const std::array<double, N>& a,
                                  const std::array<double, N>& b) {
    return zip_to_point_array(a, b, std::make_index_sequence<N>{});
}

///	centres: 0 is dc, 1 is the nyquist frequency
/// amplitudes: 0 to 1
/// delay: phase delay. 0 is probably a bad idea, 1 is ok, larger values may
///        help the estimator to design a viable filter
/// iterations: the larger this value, the more likely the result will converge,
///				but it will also take longer
template <size_t OutputOrder, size_t N>
auto arbitrary_magnitude_filter(const std::array<double, N>& centres,
                                const std::array<double, N>& amplitudes,
                                double delay,
                                size_t iterations) {
    if (!std::is_sorted(begin(centres), end(centres))) {
        throw std::runtime_error{
                "frequencies must be sorted in ascending order"};
    }

    const auto scaled_centres = map([](auto i) { return i * M_PI; }, centres);

    const auto points = zip_to_point_array(scaled_centres, amplitudes);

    constexpr auto specification_points = 100;
    std::array<double, specification_points> interp_amplitudes;
    std::array<double, specification_points> interp_frequencies;

    for (auto i = 0; i != specification_points; ++i) {
        interp_frequencies[i] = i * M_PI / specification_points;
        /// TODO If we want to interpolate a sorted list, we could go faster
        /// because we don't need to binary search each time.
        interp_amplitudes[i] = interp(begin(points),
                                      end(points),
                                      interp_frequencies[i],
                                      cosine_interp_functor{});
    }

    const auto response =
            make_response(interp_amplitudes, interp_frequencies, delay);

    ///	TODO I don't really know how to specify error weightings.
    const auto error_weighting = [&] {
        std::array<double, specification_points> ret;
        std::fill(begin(ret), end(ret), 1);
        return ret;
    }();

    return eqnerror<OutputOrder, OutputOrder>(
            interp_frequencies, response, error_weighting, iterations);
}

////////////////////////////////////////////////////////////////////////////////

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
    const auto band_centres = hrtf_data::hrtf_band_centres(sample_rate);
    const auto reflectance =
            map([](double i) { return absorption_to_pressure_reflectance(i); },
                absorption);

    for (auto delay = 0; delay != 50; ++delay) {
        const auto reflectance_coeffs = make_coefficients_canonical(
                arbitrary_magnitude_filter<coefficients_canonical::order>(
                        band_centres,
                        reflectance,
                        coefficients_canonical::order,
                        10));

        if (is_stable(reflectance_coeffs.a)) {
            return reflectance_coeffs;
        }
    }

    throw std::runtime_error{"unable to generate stable boundary filter"};
}

}  // namespace waveguide
