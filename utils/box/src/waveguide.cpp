#include "box/waveguide.h"

#include "waveguide/calibration.h"
#include "waveguide/mesh.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/callback_accumulator.h"

#include "utilities/progress_bar.h"

#include <iostream>

aligned::vector<waveguide::postprocessor::directional_receiver::output>
run_waveguide(const geo::box& box,
              float absorption,
              const glm::vec3& source,
              const glm::vec3& receiver,
              float speed_of_sound,
              float acoustic_impedance,
              float sample_rate,
              float simulation_time) {
    const compute_context cc{};
    auto voxels_and_mesh{waveguide::compute_voxels_and_mesh(
            cc,
            geo::get_scene_data(box, make_surface(absorption, 0)),
            source,
            sample_rate,
            speed_of_sound)};

    //  TODO stop using flat coefficients probably

    auto& mesh{std::get<1>(voxels_and_mesh)};
    mesh.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(absorption, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{compute_index(mesh.get_descriptor(), receiver)};

    const auto grid_spacing{mesh.get_descriptor().spacing};

    const auto calibration_factor{waveguide::rectilinear_calibration_factor(
            grid_spacing, acoustic_impedance)};

    std::cerr << "calibration factor: " << calibration_factor << '\n';

    aligned::vector<float> input_signal{static_cast<float>(calibration_factor)};
    input_signal.resize(simulation_time * sample_rate, 0.0f);

    auto prep{waveguide::preprocessor::make_hard_source(
            input_node, input_signal.begin(), input_signal.end())};

    callback_accumulator<waveguide::postprocessor::directional_receiver> post{
            mesh.get_descriptor(),
            sample_rate,
            acoustic_impedance / speed_of_sound,
            output_node};

    progress_bar pb{std::cerr, input_signal.size()};
    run(cc,
        mesh,
        prep,
        [&](auto& queue, const auto& buffer, auto step) {
            post(queue, buffer, step);
            pb += 1;
        },
        true);

    return post.get_output();
}
