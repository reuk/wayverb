#pragma once

#include "waveguide/filters.h"

#include "common/map_to_vector.h"
#include "common/scene_data.h"
#include "common/surfaces.h"

namespace waveguide {

//  TODO need better method of generating filters
//  complex curve fitting, levy

template <size_t I>
inline filter_descriptor compute_filter_descriptor(const surface& surface) {
    const auto reflectance{
            absorption_to_pressure_reflectance(surface.specular_absorption)};
    const auto gain{decibels::a2db(reflectance.s[I])};
    const auto centre{(hrtf_data::edges[I + 0] + hrtf_data::edges[I + 1]) / 2};
    //  produce a filter descriptor struct for this filter
    return {gain, centre, 1.414};
}

template <size_t... Ix>
constexpr std::array<filter_descriptor, biquad_sections> to_filter_descriptors(
        std::index_sequence<Ix...>, const surface& surface) {
    return {{compute_filter_descriptor<Ix>(surface)...}};
}

constexpr std::array<filter_descriptor, biquad_sections> to_filter_descriptors(
        const surface& surface) {
    return to_filter_descriptors(std::make_index_sequence<biquad_sections>(),
                                 surface);
}

inline coefficients_canonical to_filter_coefficients(const surface& surface,
                                                     float sr) {
    const auto descriptors{to_filter_descriptors(surface)};
    //  Transform filter parameters into a set of biquad coefficients.
    const auto individual_coeffs{get_peak_biquads_array(descriptors, sr)};
    //  Combine biquad coefficients into coefficients for a single
    //  high-order filter.
    const auto ret{convolve(individual_coeffs)};

    //  Transform from reflection filter to impedance filter.
    return to_impedance_coefficients(ret);
}

inline aligned::vector<coefficients_canonical> to_filter_coefficients(
        const aligned::vector<surface>& surfaces, float sr) {
    return map_to_vector(surfaces,
                         [=](auto i) { return to_filter_coefficients(i, sr); });
}

inline coefficients_canonical to_flat_coefficients(const surface& surface) {
    const auto reflectance{
            absorption_to_pressure_reflectance(surface.specular_absorption)};
    const coefficients_canonical ret{{reflectance.s[0]}, {1}};
    return to_impedance_coefficients(ret);
}

inline aligned::vector<coefficients_canonical> to_flat_coefficients(
        const aligned::vector<surface>& surfaces) {
    return map_to_vector(surfaces,
                         [](auto i) { return to_flat_coefficients(i); });
}

}  // namespace waveguide