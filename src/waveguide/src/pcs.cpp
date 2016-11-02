#include "waveguide/pcs.h"
#include "waveguide/config.h"

#include <cmath>
#include <numeric>

namespace wayverb {
namespace waveguide {

offset_signal maxflat(double f0, uint32_t N, double A, uint32_t hLen) {
    util::aligned::vector<double> h(hLen, 0.0f);
    const int64_t Q{2 * N - 1};
    for (auto n = -Q, end = Q + 1; n != end; ++n) {
        const auto top = std::pow(factdbl(Q), 2) * std::sin(n * 2 * M_PI * f0);
        const auto hi = factdbl(2 * N + n - 1);
        const auto lo = factdbl(2 * N - n - 1);
        const auto bot = n * hi * lo;
        h[n + Q] = top / (bot * (n % 2 ? 2 : M_PI));
    }
    h[Q] = 2 * f0;
    const auto scale_factor =
            A / std::accumulate(h.begin() + 1,
                                h.end(),
                                std::abs(h.front()),
                                [](auto i, auto j) {
                                    return std::max(i, std::abs(j));
                                });
    for (auto& i : h) {
        i *= scale_factor;
    }
    return {std::move(h), N * 2};
}

////////////////////////////////////////////////////////////////////////////////

double compute_g0(double acoustic_impedance,
                  double speed_of_sound,
                  double sample_rate,
                  double radius) {
    const auto courant_squared = 1.0 / 3;
    const auto ambient_density = acoustic_impedance / speed_of_sound;
    const auto sphere_surface_area = 4 * M_PI * radius * radius;
    const auto spatial_sample_period =
            config::grid_spacing(speed_of_sound, 1 / sample_rate);
    return courant_squared * ambient_density * sphere_surface_area /
           spatial_sample_period;
}

core::filter::biquad::coefficients mech_sphere(double M,
                                               double f0,
                                               double Q,
                                               double T) {
    const auto fs = 1 / T;
    const auto w0 = 2 * M_PI * f0 * fs;
    const auto K = M * std::pow(w0, 2);
    const auto R = w0 * M / Q;
    const auto beta = w0 / std::tan(w0 * T / 2);
    const auto den = M * std::pow(beta, 2) + R * beta + K;
    const auto b0 = beta / den;
    const auto b2 = -b0;
    const auto a1 = (2 * (K - M * std::pow(beta, 2))) / den;
    const auto a2 = 1 - (2 * R * beta / den);
    return {b0, 0, b2, a1, a2};
}

////////////////////////////////////////////////////////////////////////////////

offset_signal design_pcs_source(size_t length,
                                double acoustic_impedance,
                                double speed_of_sound,
                                double sample_rate,
                                double radius,
                                double sphere_mass,
                                double low_cutoff_hz,
                                double low_q) {
    auto pulse_shaping_filter = maxflat(0.20, 16, 1, length);
    core::filter::biquad mechanical_filter{mech_sphere(
            sphere_mass, low_cutoff_hz / sample_rate, low_q, 1 / sample_rate)};
    run_one_pass(mechanical_filter,
                 pulse_shaping_filter.signal.begin(),
                 pulse_shaping_filter.signal.end());
    const auto g0 =
            compute_g0(acoustic_impedance, speed_of_sound, sample_rate, radius);
    for (auto& samp : pulse_shaping_filter.signal) {
        samp *= g0;
    }
    const auto one_over_two_T = sample_rate / 2;
    core::filter::biquad injection_filter{
            {one_over_two_T, 0, -one_over_two_T, 0, 0}};
    run_one_pass(injection_filter,
                 pulse_shaping_filter.signal.begin(),
                 pulse_shaping_filter.signal.end());
    return pulse_shaping_filter;
}

}  // namespace waveguide
}  // namespace wayverb
