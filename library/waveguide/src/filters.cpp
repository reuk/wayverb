#include "waveguide/filters.h"

#include "utilities/foldl.h"

#include <array>

namespace waveguide {

coefficients_biquad get_peak_coefficients(const filter_descriptor& n) {
    const auto A = decibels::db2a(n.gain / 2);
    const auto w0 = 2.0 * M_PI * n.centre;
    const auto cw0 = cos(w0);
    const auto sw0 = sin(w0);
    const auto alpha = sw0 / 2.0 * n.Q;
    const auto a0 = 1 + alpha / A;
    return coefficients_biquad{
            {(1 + (alpha * A)) / a0, (-2 * cw0) / a0, (1 - alpha * A) / a0},
            {1, (-2 * cw0) / a0, (1 - alpha / A) / a0}};
}

biquad_coefficients_array get_peak_biquads_array(
        const std::array<filter_descriptor, biquad_sections>& n) {
    return get_biquads_array(get_peak_coefficients, n);
}

coefficients_canonical convolve(const biquad_coefficients_array& a) {
    return foldl([](const auto& i, const auto& j) { return convolve(i, j); },
                 a.array);
}

/// Given a set of canonical coefficients describing a reflectance filter,
/// produce an impedance filter which describes the reflective surface
coefficients_canonical to_impedance_coefficients(
        const coefficients_canonical& c) {
    coefficients_canonical ret;
    std::transform(std::begin(c.a),
                   std::end(c.a),
                   std::begin(c.b),
                   std::begin(ret.b),
                   [](auto a, auto b) { return a + b; });
    std::transform(std::begin(c.a),
                   std::end(c.a),
                   std::begin(c.b),
                   std::begin(ret.a),
                   [](auto a, auto b) { return a - b; });

    if (ret.a[0] != 0) {
        const auto norm = 1.0 / ret.a[0];
        const auto do_normalize = [norm](auto& i) {
            std::for_each(
                    std::begin(i), std::end(i), [norm](auto& i) { i *= norm; });
        };
        do_normalize(ret.b);
        do_normalize(ret.a);
    }

    return ret;
}

}  // namespace waveguide
