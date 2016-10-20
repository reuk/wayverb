#pragma once

namespace raytracer {
namespace stochastic {

auto run_attenuation(const energy_histogram& histogram,
                     const model::receiver& receiver,
                     double speed_of_sound,
                     double acoustic_impedance,
                     double room_volume,
                     double sample_rate,
                     double max_seconds) {
    const auto dirac_sequence = generate_dirac_sequence(
            speed_of_sound, room_volume, sample_rate, max_volume);
    //  TODO should be directional depending on receiver...
    return mono_diffuse_postprocessing(
            histogram, dirac_sequence, acoustic_impedance);
}

}  // namespace stochastic
}  // namespace raytracer
