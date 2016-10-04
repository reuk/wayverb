#include "common/filters_common.h"

#include "common/sinc.h"

#include <iostream>
#include <numeric>

namespace filter {

lopass_windowed_sinc::lopass_windowed_sinc(int inputLength)
        : convolver_{KERNEL_LENGTH + inputLength - 1} {}

void lopass_windowed_sinc::set_params(double co, double s) {
    const auto i{lopass_sinc_kernel(s, co, KERNEL_LENGTH)};
    proc::copy(i, kernel_.begin());
}

hipass_windowed_sinc::hipass_windowed_sinc(int inputLength)
        : convolver_{KERNEL_LENGTH + inputLength - 1} {}

void hipass_windowed_sinc::set_params(double co, double s) {
    const auto i{hipass_sinc_kernel(s, co, KERNEL_LENGTH)};
    proc::copy(i, kernel_.begin());
}

bandpass_windowed_sinc::bandpass_windowed_sinc(int inputLength)
        : convolver_{KERNEL_LENGTH + inputLength - 1} {}

void bandpass_windowed_sinc::set_params(double l, double h, double s) {
    const auto i{bandpass_sinc_kernel(s, l, h, KERNEL_LENGTH)};
    proc::copy(i, kernel_.begin());
}

//----------------------------------------------------------------------------//

biquad::coefficients compute_bandpass_biquad_coefficients(double lo,
                                                          double hi,
                                                          double sr) {
    const auto c{sqrt(lo * hi)};
    const auto omega{2 * M_PI * c / sr};
    const auto cs{cos(omega)};
    const auto sn{sin(omega)};
    const auto bandwidth{log2(hi / lo)};
    const auto Q{sn / (log(2) * bandwidth * omega)};
    const auto alpha{sn * sinh(1 / (2 * Q))};

    const auto a0{1 + alpha};
    const auto nrm{1 / a0};

    return {nrm * alpha,
            nrm * 0,
            nrm * -alpha,
            nrm * (-2 * cs),
            nrm * (1 - alpha)};
}

namespace {
double get_c(double co, double sr) {
    const auto wcT{M_PI * co / sr};
    return cos(wcT) / sin(wcT);
}
}  // namespace

biquad::coefficients compute_linkwitz_riley_lopass_coefficients(double cutoff,
                                                                double sr) {
    const auto c{get_c(cutoff, sr)};
    const auto a0{c * c + c * sqrt(2) + 1};
    return {1 / a0,
            2 / a0,
            1 / a0,
            (-2 * (c * c - 1)) / a0,
            (c * c - c * sqrt(2) + 1) / a0};
}

biquad::coefficients compute_linkwitz_riley_hipass_coefficients(double cutoff,
                                                                double sr) {
    const auto c{get_c(cutoff, sr)};
    const auto a0{c * c + c * sqrt(2) + 1};
    return {(c * c) / a0,
            (-2 * c * c) / a0,
            (c * c) / a0,
            (-2 * (c * c - 1)) / a0,
            (c * c - c * sqrt(2) + 1) / a0};
}

biquad::coefficients compute_dc_blocker_coefficients(double a) {
    return {1, -1, 0, -a, 0};
}

//----------------------------------------------------------------------------//

biquad::coefficients compute_lopass_butterworth_segment(double cf,
                                                        size_t order,
                                                        size_t segment) {
    const auto cf2{cf * cf};
    const auto p{2 * cf * std::cos(M_PI * (order + 2 * (segment + 1) - 1) /
                                   (2 * order))};
    const auto a0{1 - p + cf2};
    return {cf2 / a0,
            (2.0 * cf2) / a0,
            cf2 / a0,
            (2.0 * (cf2 - 1.0)) / a0,
            (cf2 + p + 1.0) / a0};
}

//----------------------------------------------------------------------------//

biquad::coefficients compute_hipass_butterworth_segment(double cf,
                                                        size_t order,
                                                        size_t segment) {
    const auto cf2{cf * cf};
    const auto cf3{cf * cf2};
    const auto p{2 * cf2 * std::cos(M_PI * (order + 2 * (segment + 1) - 1) /
                                    (2 * order))};
    const auto a0{cf - p + cf3};
    return {cf / a0,
            (-2.0 * cf) / a0,
            cf / a0,
            (2.0 * (cf3 - cf)) / a0,
            (cf3 + p + cf) / a0};
}

}  // namespace filter
