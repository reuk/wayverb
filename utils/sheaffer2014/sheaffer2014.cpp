#include "waveguide/mesh.h"
#include "waveguide/pcs.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/dsp_vector_ops.h"
#include "common/progress_bar.h"
#include "common/write_audio_file.h"

#include <iostream>

/// Attempts to replicate the physically modelled source tests from the paper
/// 'physical and numerical constraints in source modeling for finite
/// difference simulation of room acoustics' by Jonathan Sheaffer, Maarten van
/// Walstijn, and Brun Fazenda.

int main() {
    //  Set up all the generic features of the simulation.

    const geo::box box{glm::vec3{-3}, glm::vec3{3}};
    const auto sample_rate{16000.0};
    const auto speed_of_sound{340.0};
    const auto acoustic_impedance{400.0};

    const compute_context cc{};

    const glm::vec3 receiver{0};
    const auto radius{1.5};
    const glm::vec3 source{
            std::sin(M_PI / 4) * radius, 0, std::cos(M_PI / 4) * radius};

    auto voxels_and_mesh{
            waveguide::compute_voxels_and_mesh(cc,
                                               geo::get_scene_data(box),
                                               receiver,
                                               sample_rate,
                                               speed_of_sound)};

    auto& mesh{std::get<1>(voxels_and_mesh)};
    mesh.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(0.1, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{compute_index(mesh.get_descriptor(), receiver)};

    //  Now we get to do the interesting new bit:
    //  Set up a physically modelled source signal.

    const auto write{[=](auto name, auto sig) {
        snd::write(build_string(name, ".wav"), {sig}, sample_rate, 16);
        normalize(sig);
        snd::write(build_string("normalised.", name, ".wav"),
                   {sig},
                   sample_rate,
                   16);
    }};

    auto pulse_shaping_filter{maxflat(0.075, 16, 250e-6, 1 << 12)};
    write("pulse_shaping_filter", pulse_shaping_filter);

    {
        filter::biquad mechanical_filter{
                mech_sphere(0.025, 100 / sample_rate, 0.7, 1 / sample_rate)};
        run_one_pass(mechanical_filter,
                     pulse_shaping_filter.begin(),
                     pulse_shaping_filter.end());
    }
    write("transduced", pulse_shaping_filter);

    const auto one_over_two_T{sample_rate / 2};

    {
        filter::biquad injection_filter{
                {one_over_two_T, 0, -one_over_two_T, 0, 0}};
        run_one_pass(injection_filter,
                     pulse_shaping_filter.begin(),
                     pulse_shaping_filter.end());
    }

    write("injected", pulse_shaping_filter);

    //  Now run the actual simulation.

    auto input_signal{pulse_shaping_filter};
    input_signal.resize(1 << 16, 0);

    progress_bar pb{std::cerr, input_signal.size()};
    const auto results{waveguide::run(cc,
                                      mesh,
                                      input_node,
                                      input_signal.begin(),
                                      input_signal.end(),
                                      output_node,
                                      speed_of_sound,
                                      acoustic_impedance,
                                      [&](auto i) { pb += 1; })};

    const auto pressures{
            map_to_vector(results, [](auto i) { return i.pressure; })};
    write("output", pressures);

    return EXIT_SUCCESS;
}
