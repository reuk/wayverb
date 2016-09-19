#include "waveguide/pcs.h"

#include <cmath>
#include <numeric>

aligned::vector<double> maxflat(double f0,
                                uint32_t N,
                                double A,
                                uint32_t hLen) {
    aligned::vector<double> h(hLen, 0.0f);
    const int64_t Q{2 * N - 1};
    for (auto n{-Q}, end{Q + 1}; n != end; ++n) {
        const auto top{std::pow(factdbl(Q), 2) * std::sin(n * 2 * M_PI * f0)};
        const auto hi{factdbl(2 * N + n - 1)};
        const auto lo{factdbl(2 * N - n - 1)};
        const auto bot{n * hi * lo};
        h[n + Q] = top / (bot * (n % 2 ? 2 : M_PI));
    }
    h[Q] = 2 * f0;
    const auto scale_factor{A / std::accumulate(h.begin() + 1,
                                                h.end(),
                                                std::abs(h.front()),
                                                [](auto i, auto j) {
                                                    return std::max(
                                                            i, std::abs(j));
                                                })};
    for (auto& i : h) {
        i *= scale_factor;
    }
    return h;
}

//----------------------------------------------------------------------------//

filter::biquad::coefficients mech_sphere(double M,
                                         double f0,
                                         double Q,
                                         double T) {
    const auto fs{1 / T};
    const auto w0{2 * M_PI * f0 * fs};
    const auto K{M * std::pow(w0, 2)};
    const auto R{w0 * M / Q};
    const auto beta{w0 / std::tan(w0 * T / 2)};
    const auto den{M * std::pow(beta, 2) + R * beta + K};
    const auto b0{beta / den};
    const auto b2{-b0};
    const auto a1{(2 * (K - M * std::pow(beta, 2))) / den};
    const auto a2{1 - (2 * R * beta / den)};
    return {b0, 0, b2, a1, a2};
}

//----------------------------------------------------------------------------//

aligned::vector<double> pcs_design(
        double M, double f0, double Q, double maxF, double fc, double T) {
    auto h{maxflat(fc, 16, maxF, 1 << 12)};
    filter::biquad filt{mech_sphere(M, f0, Q, T)};
    run_one_pass(filt, h.begin(), h.end());
    return h;
}
