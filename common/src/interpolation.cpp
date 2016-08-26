#include "common/interpolation.h"

#include "common/hrtf.h"

/// This is rubbish but I don't have time to write a proper cubic spline
/// interpolation.
/// y0 and y1 are interpolation values, x is between 0 and 1
float cosine_interpolation(float y0, float y1, float x) {
    const auto phase{(1 - cos(x * M_PI)) * 0.5};
    return y0 * (1 - phase) + y1 * phase;
}

std::array<float, 128> smooth_energy_bands(const volume_type& v,
                                           double sample_rate) {
    std::array<float, 10> centre_frequencies;
    centre_frequencies.front() = 20;
    for (auto i{1u}; i != centre_frequencies.size() - 1; ++i) {
        centre_frequencies[i] =
                (hrtf_data::edges[i] + hrtf_data::edges[i + 1]) / 2;
    }
    centre_frequencies.back() = 20000;

    std::array<float, 128> ret;
    auto this_band{1u};
    for (auto i{0u}; i != ret.size(); ++i) {
        const auto bin_frequency{i * sample_rate / 2 * ret.size()};
        while (centre_frequencies[this_band] < bin_frequency) {
            this_band += 1;
        }

        const auto lower{centre_frequencies[this_band - 1]};
        const auto upper{centre_frequencies[this_band]};
        const auto x{(bin_frequency - lower) / (upper - lower)};

        ret[i] = cosine_interpolation(lower, upper, x);
    }
    
    return ret;
}
