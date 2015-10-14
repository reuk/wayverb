#pragma once

#include "filters.h"
#include "logger.h"

#include <vector>

template<typename T>
auto gcd(T a, T b) {
    while (b != 0) {
        auto t = b;
        b = a % b;
        a = t;
    }
    return a;
}

template<typename T>
auto intersperse(const std::vector<T> & t, int spacing) {
    std::vector<T> ret(t.size() * spacing, T(0));
    for (auto i = 0u; i != t.size(); ++i)
        ret[i * spacing] = t[i];
    return ret;
}

template<typename T>
auto outersperse(const std::vector<T> & t, int spacing) {
    std::vector<T> ret(t.size() / spacing);
    for (auto i = 0u; i != ret.size(); ++i)
        ret[i] = t[i * spacing];
    return ret;
}

class SampleRateConversionFilter : public FastConvolution {
public:
    SampleRateConversionFilter(unsigned long kernel_length, unsigned long signal_length)
            : FastConvolution (kernel_length + signal_length - 1)
            , kernel(windowed_sinc_kernel(0.5, kernel_length)) {
        Logger::log("kernel size: ", kernel.size());
    }
    virtual ~SampleRateConversionFilter() noexcept = default;

    auto operator() (const std::vector<float> & signal) {
        LOG_SCOPE;
        return convolve(signal, kernel);
    }

private:
    std::vector<float> kernel;
};

auto convert_sample_rate(const std::vector<float> & t, int numerator, int denominator) {
    auto common = gcd(numerator, denominator);
    numerator /= common;
    denominator /= common;
    auto interspersed = intersperse(t, numerator);
    auto filtered = SampleRateConversionFilter(127, interspersed.size())(interspersed);
    return outersperse(filtered, denominator);
}
