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
            {waveguide::to_flat_coefficients(make_surface(0.005991, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{compute_index(mesh.get_descriptor(), receiver)};

    //  Now we get to do the interesting new bit:
    //  Set up a physically modelled source signal.

    const auto input_signal{waveguide::design_pcs_source(1 << 13, sample_rate)};

    //  Run the simulation.

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

    const auto write{[=](auto name, auto sig) {
        snd::write(build_string(name, ".wav"), {sig}, sample_rate, 16);
        normalize(sig);
        snd::write(build_string("normalised.", name, ".wav"),
                   {sig},
                   sample_rate,
                   16);
    }};

    write("output", pressures);

    return EXIT_SUCCESS;
}
