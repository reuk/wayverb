#pragma once

#include <cstdlib>

namespace waveguide {

struct single_band_parameters final {
    /// The actual sampling frequency of the waveguide mesh in Hz.
    /// The maximum 'valid' frequency in the mesh is 0.25 of this value,
    /// although in practice the error is unreasonable above 0.15 of the
    /// sampling frequency.
    double sample_rate;

    /// The proportion of the 'valid' spectrum that should be used.
    /// Values between 0 and 1 are valid, but 0.6 or lower is recommended.
    /// The cutoff filter frequency is found by doing
    /// sample_rate * usable_portion * 0.25;
    double usable_portion;
};

constexpr auto compute_cutoff_frequency(double sample_rate,
                                        double usable_portion) {
    return sample_rate * 0.25 * usable_portion;
}

constexpr auto compute_sampling_frequency(double cutoff,
                                          double usable_portion) {
    return cutoff / (0.25 * usable_portion);
}

struct multiple_band_variable_spacing_parameters final {
    /// The number of bands which should be simulated with the waveguide.
    /// Be careful with high numbers.
    /// Not only will the waveguide be run once for each band, but the
    /// simulated frequency will increase with each band, so that the
    /// simulation will take *much* longer.
    size_t bands;

    /// Same as the usable_portion of the single_band_parameters.
    /// The waveguide simulation sampling rate is found by taking the high
    /// edge of the frequency band, and dividing it by usable_portion * 0.25.
    double usable_portion;
};

struct multiple_band_constant_spacing_parameters final {
    /// The number of bands which should be simulated with the waveguide.
    /// Be careful with high numbers.
    /// The waveguide will be run once, at the specified sampling rate, for each
    /// band, so i.e. 4 bands will take 4 times as long.
    size_t bands;
    
    /// The sample rate to use for all bands.
    double sample_rate;

    /// As above.
    double usable_portion;
};

}  // namespace waveguide
