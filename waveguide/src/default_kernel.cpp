#include "waveguide/default_kernel.h"

#include "waveguide/make_transparent.h"

#include "common/kernel.h"

#include <cassert>

default_kernel::default_kernel(double sampling_frequency)
        : default_kernel(kernels::ricker_kernel(sampling_frequency)) {
//        : default_kernel(kernels::sin_modulated_gaussian_kernel(sampling_frequency)) {
}

default_kernel::default_kernel(aligned::vector<float>&&opaque)
    : kernel(make_transparent(opaque))
    , opaque_kernel_size(opaque.size())
    , correction_offset_in_samples(opaque.size() / 2)
{
}
