#include "common/filters_common.h"

#include "common/sinc.h"

#include <iostream>
#include <numeric>

namespace filter {

void Lopass::set_params(float co, float s) {
    cutoff = co;
    sr = s;
}

void Hipass::set_params(float co, float s) {
    cutoff = co;
    sr = s;
}

void Bandpass::set_params(float l, float h, float s) {
    lo = l;
    hi = h;
    sr = s;
}

LopassWindowedSinc::LopassWindowedSinc(int inputLength)
        : FastConvolution(KERNEL_LENGTH + inputLength - 1) {
}

void LopassWindowedSinc::filter(std::vector<float> &data) {
    data = convolve(kernel, data);
}

void LopassWindowedSinc::set_params(float co, float s) {
    auto i = lopass_sinc_kernel(s, co, KERNEL_LENGTH);
    proc::copy(i, kernel.begin());
}

HipassWindowedSinc::HipassWindowedSinc(int inputLength)
        : FastConvolution(KERNEL_LENGTH + inputLength - 1) {
}

void HipassWindowedSinc::filter(std::vector<float> &data) {
    data = convolve(kernel, data);
}

void HipassWindowedSinc::set_params(float co, float s) {
    auto i = hipass_sinc_kernel(s, co, KERNEL_LENGTH);
    proc::copy(i, kernel.begin());
}

BandpassWindowedSinc::BandpassWindowedSinc(int inputLength)
        : FastConvolution(KERNEL_LENGTH + inputLength - 1) {
}

void BandpassWindowedSinc::filter(std::vector<float> &data) {
    data = convolve(kernel, data);
}

void BandpassWindowedSinc::set_params(float l, float h, float s) {
    auto i = bandpass_sinc_kernel(s, l, h, KERNEL_LENGTH);
    proc::copy(i, kernel.begin());
}

void Biquad::filter(std::vector<float> &data) {
    double z1 = 0;
    double z2 = 0;

    for (auto &&i : data) {
        double out = i * b0 + z1;
        z1 = i * b1 + z2 - a1 * out;
        z2 = i * b2 - a2 * out;
        i = out;
    }
}

void Biquad::set_params(
        double _b0, double _b1, double _b2, double _a1, double _a2) {
    b0 = _b0;
    b1 = _b1;
    b2 = _b2;
    a1 = _a1;
    a2 = _a2;
}

void BandpassBiquad::set_params(float lo, float hi, float sr) {
    // From www.musicdsp.org/files/Audio-EQ-Cookbook.txt
    const double c = sqrt(lo * hi);
    const double omega = 2 * M_PI * c / sr;
    const double cs = cos(omega);
    const double sn = sin(omega);
    const double bandwidth = log2(hi / lo);
    const double Q = sn / (log(2) * bandwidth * omega);
    const double alpha = sn * sinh(1 / (2 * Q));

    const double a0 = 1 + alpha;
    const double nrm = 1 / a0;

    Biquad::set_params(nrm * alpha,
                       nrm * 0,
                       nrm * -alpha,
                       nrm * (-2 * cs),
                       nrm * (1 - alpha));
}

double getC(double co, double sr) {
    const double wcT = M_PI * co / sr;
    return cos(wcT) / sin(wcT);
}

void LinkwitzRileySingleLopass::set_params(float cutoff, float sr) {
    Lopass::set_params(cutoff, sr);
    const double c = getC(cutoff, sr);
    const double a0 = c * c + c * sqrt(2) + 1;
    Biquad::set_params(1 / a0,
                       2 / a0,
                       1 / a0,
                       (-2 * (c * c - 1)) / a0,
                       (c * c - c * sqrt(2) + 1) / a0);
}

void LinkwitzRileySingleHipass::set_params(float cutoff, float sr) {
    Hipass::set_params(cutoff, sr);
    const double c = getC(cutoff, sr);
    const double a0 = c * c + c * sqrt(2) + 1;
    Biquad::set_params((c * c) / a0,
                       (-2 * c * c) / a0,
                       (c * c) / a0,
                       (-2 * (c * c - 1)) / a0,
                       (c * c - c * sqrt(2) + 1) / a0);
}

void LinkwitzRileyBandpass::set_params(float l, float h, float s) {
    lopass.set_params(h, s);
    hipass.set_params(l, s);
}

void LinkwitzRileyBandpass::filter(std::vector<float> &data) {
    lopass.filter(data);
    hipass.filter(data);
}

DCBlocker::DCBlocker() {
    Biquad::set_params(1, 1, 0, R, 0);
}
}  // namespace filter
