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
              const model::parameters& params,
              float sample_rate,
              float simulation_time) {
    const compute_context cc{};
    auto voxels_and_mesh{waveguide::compute_voxels_and_mesh(
            cc,
            geo::get_scene_data(box, make_surface(absorption, 0)),
            params.source,
            sample_rate,
            params.speed_of_sound)};

    //  TODO stop using flat coefficients probably

    auto& mesh{std::get<1>(voxels_and_mesh)};
    mesh.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(absorption, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), params.source)};
    const auto output_node{
            compute_index(mesh.get_descriptor(), params.receiver)};

    const auto grid_spacing{mesh.get_descriptor().spacing};

    const auto calibration_factor{waveguide::rectilinear_calibration_factor(
            grid_spacing, params.acoustic_impedance)};

    std::cerr << "calibration factor: " << calibration_factor << '\n';

    aligned::vector<float> input_signal{static_cast<float>(calibration_factor)};
    input_signal.resize(simulation_time * sample_rate, 0.0f);

    auto prep{waveguide::preprocessor::make_hard_source(
            input_node, input_signal.begin(), input_signal.end())};

    callback_accumulator<waveguide::postprocessor::directional_receiver> post{
            mesh.get_descriptor(),
            sample_rate,
            params.acoustic_impedance / params.speed_of_sound,
            output_node};

    progress_bar pb;
    run(cc,
        mesh,
        prep,
        [&](auto& queue, const auto& buffer, auto step) {
            post(queue, buffer, step);
            set_progress(pb, step, input_signal.size());
        },
        true);

    return post.get_output();
}
