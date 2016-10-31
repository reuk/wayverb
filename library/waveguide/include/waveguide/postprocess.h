#pragma once

#include "waveguide/attenuator.h"
#include "waveguide/canonical.h"
#include "waveguide/config.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/cl/iterator.h"
#include "common/cl/scene_structs.h"
#include "common/mixdown.h"
#include "common/model/receiver.h"

#include "utilities/map_to_vector.h"

namespace waveguide {

template <typename T>
using dereferenced_t = decltype(*std::declval<T>());

template <typename T, typename U>
constexpr auto dereferenced_type_matches_v =
        std::is_same<std::decay_t<dereferenced_t<T>>, U>::value;

/// We need a unified interface for dealing with single-band microphone output
/// and multi-band hrtf output.
/// We try to determine whether the iterator is over a single- or multi-band
/// type, and then filter appropriately in the multi-band case.

/// We start with the 'audible range' defined in Hz.
/// This gives us well-defined 8-band edges and widths, also in Hz.

/// If the iterator is over `bands_type` use this one.
/// Audible range is normalised in terms of the waveguide sampling rate.
template <typename It,
          std::enable_if_t<std::is_same<std::decay_t<dereferenced_t<It>>,
                                        bands_type>::value,
                           int> = 0>
auto postprocess(It begin, It end, double sample_rate) {
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
auto postprocess(It begin, It end, double sample_rate) {
    return aligned::vector<float>(begin, end);
}

////////////////////////////////////////////////////////////////////////////////

template <typename Method>
auto postprocess(const band& band,
                 double usable_portion,
                 const Method& method,
                 double acoustic_impedance,
                 double output_sample_rate) {
    auto attenuated =
            map_to_vector(begin(band.directional),
                          end(band.directional),
                          make_attenuate_mapper(method, acoustic_impedance));
    const auto ret =
            postprocess(begin(attenuated), end(attenuated), band.sample_rate);
    return waveguide::adjust_sampling_rate(ret.data(),
                                           ret.data() + ret.size(),
                                           band.sample_rate,
                                           output_sample_rate);
}

template <typename Method>
auto postprocess(const simulation_results& results,
                 const Method& method,
                 double acoustic_impedance,
                 double output_sample_rate) {
    aligned::vector<float> ret;

    for (const auto& band : results.bands) {
        const auto processed = postprocess(band,
                                           results.usable_portion,
                                           method,
                                           acoustic_impedance,
                                           output_sample_rate);

        //  TODO Remove this line.
        throw std::runtime_error{"not implemented"};

        //  TODO For the lowest band, lopass based on sampling rate and usable
        //  portion.
        //  For other bands, bandpass based on previous band cutoff.
        
        //  TODO Add results to ret.
    }

    return ret;
}

}  // namespace waveguide
