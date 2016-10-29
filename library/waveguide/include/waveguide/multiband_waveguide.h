#pragma once

#include "waveguide/waveguide.h"

/// Instead of running the waveguide once with complex boundaries, we run it
/// several times with different 'flat' boundaries and max frequencies, then
/// filter into a final response.

namespace waveguide {

auto multiband_run(
        const compute_context& cc,
        const generic_scene_data<cl_float3, surface<simulation_bands>>& scene,
        double oversample,
        double speed_of_sound) {
    if (oversample < 1) {
        throw std::runtime_error{
                "oversample must be greater than or equal to 1, although 2 or "
                "greater is recommended"};
    }

    const auto band_centres_hz = hrtf_data::hrtf_band_centres_hz();

    //  For each band, up until some maximum.
    for (auto band = 0; band != max_bands; ++band) {
        //  Calculate the appropriate waveguide sampling rate.
        const auto max_frequency_hz = band_centres_hz[band];
        const auto waveguide_sample_rate =
                centre_frequency_hz * oversampling / 0.25;

        //  TODO Determine anchor position.

        //  Compute mesh using correct grid spacing for this frequency.
        const auto voxels_and_mesh = compute_voxels_and_mesh(
                cc, scene, anchor, waveguide_sample_rate, speed_of_sound);

        //  TODO Set mesh boundaries with correct response for this frequency band.

        //  TODO Set up input/output objects.
        
        //  TODO Run waveguide.

        //  TODO Collect results.
    }

    //  TODO Return per-band results.
}

}//namespace waveguide
