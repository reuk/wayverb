#pragma once

#include "common/mixdown.h"

namespace waveguide {

template <typename T>
using dereferenced_t = decltype(*std::declval<T>());

template <typename T, typename U>
constexpr auto dereferenced_type_matches_v{
        std::is_same<std::decay_t<dereferenced_t<T>>, U>::value};

/// We need a unified interface for dealing with single-band microphone output
/// and multi-band hrtf output.
/// We try to determine whether the iterator is over a single- or multi-band
/// type, and then filter appropriately in the multi-band case.

/// We start with the 'audible range' defined in Hz.
/// This gives us well-defined 8-band edges and widths, also in Hz.

/// If the iterator is over `volume_type` use this one.
/// Audible range is normalised in terms of the waveguide sampling rate.
template <typename It,
          std::enable_if_t<std::is_same<std::decay_t<dereferenced_t<It>>,
                                        volume_type>::value,
                           int> = 0>
auto multiband_process(It begin, It end, double sample_rate) {
    return multiband_filter_and_mixdown(
            begin, end, sample_rate, [](auto it, auto index) {
                return make_cl_type_iterator(std::move(it), index);
            });
}

/// If the iterator is over a floating-point type use this one.
template <typename It,
          std::enable_if_t<std::is_floating_point<
                                   std::decay_t<dereferenced_t<It>>>::value,
                           int> = 0>
auto multiband_process(It begin, It end, double sample_rate) {
    return aligned::vector<float>(begin, end);
}

}//namespace waveguide
