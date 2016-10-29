#pragma once

#include "raytracer/postprocess.h"
#include "waveguide/postprocess.h"

namespace wayverb {

struct waveguide_results final {
    aligned::vector<waveguide::postprocessor::directional_receiver::output>
            directional;
    double sample_rate;
};

template <typename Histogram>
struct combined_results final {
    raytracer::aural_results<Histogram> raytracer;
    waveguide_results waveguide;
};

template <typename Histogram>
auto make_combined_results(raytracer::aural_results<Histogram> raytracer,
                           waveguide_results waveguide) {
    return combined_results<Histogram>{std::move(raytracer),
                                       std::move(waveguide)};
}

////////////////////////////////////////////////////////////////////////////////

template <typename Histogram, typename Method>
auto postprocess(const combined_results<Histogram>& input,
                 const Method& method,
                 const glm::vec3& receiver_position,
                 double room_volume,
                 double acoustic_impedance,
                 double speed_of_sound,
                 double output_sample_rate) {
    //  Individual processing.
    const auto raytracer_processed = raytracer::postprocess(input.raytracer,
                                                            method,
                                                            receiver_position,
                                                            room_volume,
                                                            acoustic_impedance,
                                                            speed_of_sound,
                                                            output_sample_rate);

    const auto waveguide_processed =
            waveguide::postprocess(begin(input.waveguide.directional),
                                   end(input.waveguide.directional),
                                   method,
                                   acoustic_impedance,
                                   input.waveguide.sample_rate);

    //  TODO crossover filtering

    //  TODO samplerate conversion

    //  TODO final mixdown

    throw std::runtime_error{"not implemented"};
    return aligned::vector<float>{};
}

}  // namespace wayverb
