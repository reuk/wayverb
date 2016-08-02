#pragma once

#include "common/aligned/vector.h"

namespace waveguide {

/// Create a transparent source for use with a soft-source waveguide.
/// Source is a ricker wavelet with upper frequency at approximately
/// 1/4 the sampling frequency, and an appropriate length.
class default_kernel {
public:
    /// Construct the kernel.
    default_kernel(double sampling_frequency);

    /// The waveguide kernel.
    const aligned::vector<float> kernel;

    /// The length of the kernel before being made transparent.
    const size_t opaque_kernel_size;

    /// The amount by which the waveguide output should be shifted in order to
    /// place the wavefront peak at the correct time.
    const size_t correction_offset_in_samples;

private:
    default_kernel(aligned::vector<float>&& opaque);
};

}  // namespace waveguide
