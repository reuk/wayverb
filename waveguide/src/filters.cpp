#include "waveguide/filters.h"
#include "common/reduce.h"

#include <array>

namespace waveguide {

biquad_coefficients get_peak_coefficients(const descriptor& n, double sr) {
    const auto A = decibels::db2a(n.gain / 2);
    const auto w0 = 2.0 * M_PI * n.centre / sr;
    const auto cw0 = cos(w0);
    const auto sw0 = sin(w0);
    const auto alpha = sw0 / 2.0 * n.Q;
    const auto a0 = 1 + alpha / A;
    return biquad_coefficients{
            {(1 + (alpha * A)) / a0, (-2 * cw0) / a0, (1 - alpha * A) / a0},
            {1, (-2 * cw0) / a0, (1 - alpha / A) / a0}};
}

biquad_coefficients_array get_peak_biquads_array(
        const std::array<descriptor, biquad_sections>& n, double sr) {
    return get_biquads_array(n, sr, get_peak_coefficients);
}

canonical_coefficients convolve(const biquad_coefficients_array& a) {
    std::array<biquad_coefficients, biquad_sections> t;
    proc::copy(a.array, t.begin());
    return reduce(t,
                  [](const auto& i, const auto& j) { return convolve(i, j); });
}

/// Given a set of canonical coefficients describing a reflectance filter,
/// produce an impedance filter which describes the reflective surface
canonical_coefficients to_impedance_coefficients(
        const canonical_coefficients& c) {
    canonical_coefficients ret;
    proc::transform(
            c.a, std::begin(c.b), std::begin(ret.b), [](auto a, auto b) {
                return a + b;
            });
    proc::transform(
            c.a, std::begin(c.b), std::begin(ret.a), [](auto a, auto b) {
                return a - b;
            });

    if (ret.a[0] != 0) {
        const auto norm = 1.0 / ret.a[0];
        const auto do_normalize = [norm](auto& i) {
            proc::for_each(i, [norm](auto& i) { i *= norm; });
        };
        do_normalize(ret.b);
        do_normalize(ret.a);
    }

    return ret;
}

}  // namespace waveguide
