#pragma once

#include "waveguide/filters.h"

#include "common/scene_data.h"
#include "common/surfaces.h"

#include "utilities/map_to_vector.h"

#include <array>

namespace waveguide {

//  TODO need better method of generating filters
//  complex curve fitting, levy

template <size_t I>
inline auto compute_filter_descriptor(const surface<I>& surface) {
    throw std::runtime_error{
            "compute_filter_descriptor: this idea was doomed from the "
            "beginning"};
    return filter_descriptor{};
    /*
    const auto reflectance{
            absorption_to_pressure_reflectance(surface.specular_absorption)};
    const auto gain{decibels::a2db(reflectance.s[I])};
    const auto centre{(hrtf_data::edges[I + 0] + hrtf_data::edges[I + 1]) / 2};
    //  produce a filter descriptor struct for this filter
    return filter_descriptor{gain, centre, 1.414};
    */
}

/*
template <size_t... Ix>
constexpr auto to_filter_descriptors(std::index_sequence<Ix...>,
                                     const surface& surface) {
    return std::array<filter_descriptor, sizeof...(Ix)>{
            {compute_filter_descriptor<Ix>(surface)...}};
}

constexpr auto to_filter_descriptors(const surface& surface) {
    return to_filter_descriptors(std::make_index_sequence<biquad_sections>{},
                                 surface);
}

inline coefficients_canonical to_filter_coefficients(const surface& surface,
                                                     float sr) {
    const auto descriptors = to_filter_descriptors(surface);
    //  Transform filter parameters into a set of biquad coefficients.
    const auto individual_coeffs = get_peak_biquads_array(descriptors, sr);
    //  Combine biquad coefficients into coefficients for a single
    //  high-order filter.
    const auto ret = convolve(individual_coeffs);

    //  Transform from reflection filter to impedance filter.
    return to_impedance_coefficients(ret);
}

inline aligned::vector<coefficients_canonical> to_filter_coefficients(
        const aligned::vector<surface>& surfaces, float sr) {
    return map_to_vector(begin(surfaces), end(surfaces), [=](auto i) {
        return to_filter_coefficients(i, sr);
    });
}
*/

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

}  // namespace waveguide
