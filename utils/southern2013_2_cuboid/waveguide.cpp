#include "waveguide.h"

#include "waveguide/make_transparent.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/progress_bar.h"

#include <iostream>

waveguide_test::waveguide_test(const scene_data& sd,
                               float speed_of_sound,
                               float acoustic_impedance)
        : voxels_and_mesh_{waveguide::compute_voxels_and_mesh(compute_context_,
                                                              sd,
                                                              glm::vec3{0},
                                                              sample_rate_,
                                                              speed_of_sound)}
        , speed_of_sound_{speed_of_sound}
        , acoustic_impedance_{acoustic_impedance} {}

audio waveguide_test::operator()(const surface& surface,
                                 const glm::vec3& source,
                                 const model::receiver_settings& receiver) {
    auto& mesh{std::get<1>(voxels_and_mesh_)};
    mesh.set_coefficients(waveguide::to_filter_coefficients(
            aligned::vector<struct surface>{surface}, sample_rate_));

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{
            compute_index(mesh.get_descriptor(), receiver.position)};

    const aligned::vector<float> dry_signal{1};
    const auto input_signal{waveguide::make_transparent(dry_signal)};

    progress_bar pb{std::cout, input_signal.size()};
    const auto results{waveguide::run(compute_context_,
                                      mesh,
                                      input_node,
                                      input_signal,
                                      output_node,
                                      speed_of_sound_,
                                      acoustic_impedance_,
                                      [&](auto i) { pb += 1; })};

    const auto output{
            map_to_vector(results, [](auto i) { return i.pressure; })};

    return {output, sample_rate_, "waveguide"};
}
