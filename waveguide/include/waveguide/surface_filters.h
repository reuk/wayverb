#pragma once

#include "common/scene_data.h"

#include "waveguide/filters.h"

namespace waveguide {

template <size_t I>
inline filter_descriptor compute_filter_descriptor(const surface& surface) {
    const auto gain{decibels::a2db(surface.specular.s[I])};
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
    const auto descriptors = to_filter_descriptors(surface);
    //  transform filter parameters into a set of biquad coefficients
    const auto individual_coeffs = get_peak_biquads_array(descriptors, sr);
    //  combine biquad coefficients into coefficients for a single
    //  high-order
    //  filter
    const auto ret = convolve(individual_coeffs);

    //  transform from reflection filter to impedance filter
    return to_impedance_coefficients(ret);
}

inline aligned::vector<coefficients_canonical> to_filter_coefficients(
        aligned::vector<surface> surfaces, float sr) {
    aligned::vector<coefficients_canonical> ret(surfaces.size());
    proc::transform(surfaces, ret.begin(), [sr](auto i) {
        return to_filter_coefficients(i, sr);
    });
    return ret;
}

}  // namespace waveguide
