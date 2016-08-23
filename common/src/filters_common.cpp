#include "common/filters_common.h"

#include "common/sinc.h"

#include <iostream>
#include <numeric>

namespace filter {

LopassWindowedSinc::LopassWindowedSinc(int inputLength)
        : convolver(KERNEL_LENGTH + inputLength - 1) {}

void LopassWindowedSinc::set_params(double co, double s) {
    const auto i{lopass_sinc_kernel(s, co, KERNEL_LENGTH)};
    proc::copy(i, kernel.begin());
}

HipassWindowedSinc::HipassWindowedSinc(int inputLength)
        : convolver(KERNEL_LENGTH + inputLength - 1) {}

void HipassWindowedSinc::set_params(double co, double s) {
    const auto i{hipass_sinc_kernel(s, co, KERNEL_LENGTH)};
    proc::copy(i, kernel.begin());
}

BandpassWindowedSinc::BandpassWindowedSinc(int inputLength)
        : convolver(KERNEL_LENGTH + inputLength - 1) {}

void BandpassWindowedSinc::set_params(double l, double h, double s) {
    const auto i{bandpass_sinc_kernel(s, l, h, KERNEL_LENGTH)};
    proc::copy(i, kernel.begin());
}

//----------------------------------------------------------------------------//

Biquad::Biquad(double b0, double b1, double b2, double a1, double a2)
        : b0(b0)
        , b1(b1)
        , b2(b2)
        , a1(a1)
        , a2(a2) {}

double Biquad::filter(double i) {
    const auto out{i * b0 + z1};
    z1 = i * b1 + z2 - a1 * out;
    z2 = i * b2 - a2 * out;
    return out;
}

void Biquad::clear() { z1 = z2 = 0; }

//----------------------------------------------------------------------------//

Biquad make_bandpass_biquad(double lo, double hi, double sr) {
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
double getC(double co, double sr) {
    const auto wcT{M_PI * co / sr};
    return cos(wcT) / sin(wcT);
}
}  // namespace

Biquad make_linkwitz_riley_lopass(double cutoff, double sr) {
    const auto c{getC(cutoff, sr)};
    const auto a0{c * c + c * sqrt(2) + 1};
    return {1 / a0,
            2 / a0,
            1 / a0,
            (-2 * (c * c - 1)) / a0,
            (c * c - c * sqrt(2) + 1) / a0};
}

Biquad make_linkwitz_riley_hipass(double cutoff, double sr) {
    const auto c{getC(cutoff, sr)};
    const auto a0{c * c + c * sqrt(2) + 1};
    return {(c * c) / a0,
            (-2 * c * c) / a0,
            (c * c) / a0,
            (-2 * (c * c - 1)) / a0,
            (c * c - c * sqrt(2) + 1) / a0};
}

Biquad make_dc_blocker() { return {1, 1, 0, 0.995, 0}; }

}  // namespace filter
