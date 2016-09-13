#include "waveguide/default_kernel.h"

#include "waveguide/make_transparent.h"

#include "common/kernel.h"

#include <cassert>

namespace waveguide {

#define THE_KERNEL_FUNCTION kernels::ricker_kernel
//#define THE_KERNEL_FUNCTION kernels::sin_modulated_gaussian_kernel

default_kernel::default_kernel(double sampling_frequency, double valid_portion)
        : default_kernel(
                  THE_KERNEL_FUNCTION(sampling_frequency, valid_portion)) {}

default_kernel::default_kernel(aligned::vector<float>&& opaque)
        : kernel(make_transparent(opaque))
        , opaque_kernel_size(opaque.size())
        , correction_offset_in_samples(opaque.size() / 2) {}

}  // namespace waveguide
