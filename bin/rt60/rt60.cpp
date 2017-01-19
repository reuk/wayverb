#include "audio_file/audio_file.h"

#include "utilities/named_value.h"
#include "utilities/progress_bar.h"
#include "utilities/string_builder.h"

#include "core/attenuator/null.h"
#include "core/callback_accumulator.h"
#include "core/cl/common.h"
#include "core/dsp_vector_ops.h"
#include "core/environment.h"
#include "core/geo/box.h"
#include "core/reverb_time.h"

#include "raytracer/hit_rate.h"
#include "raytracer/simulation_parameters.h"

#include "waveguide/fitted_boundary.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/simulation_parameters.h"
#include "waveguide/waveguide.h"

#include "combined/engine.h"
#include "combined/waveguide_base.h"

#include <iomanip>
#include <iostream>

int main(int /*argc*/, char** /*argv*/) {
    const util::aligned::vector<util::named_value<wayverb::core::geo::box>>
            boxes{util::make_named_value(
                          "small",
                          wayverb::core::geo::box{glm::vec3{0, 0, 0},
                                                  glm::vec3{2, 2.5, 3}}),
                  util::make_named_value(
                          "medium",
                          wayverb::core::geo::box{glm::vec3{0, 0, 0},
                                                  glm::vec3{4.5, 2.5, 3.5}}),
                  util::make_named_value(
                          "large",
                          wayverb::core::geo::box{glm::vec3{0, 0, 0},
                                                  glm::vec3{12, 4, 8}})};

    const auto absorption = 0.1f;
    const auto scattering = 0.1f;
    const auto surface =
            wayverb::core::make_surface<wayverb::core::simulation_bands>(
                    absorption, scattering);

    const wayverb::core::environment environment{};
    const wayverb::core::compute_context cc{};

    const auto cutoff = 500.0;
    const auto usable = 0.6;
    const auto sample_rate =
            wayverb::waveguide::compute_sampling_frequency(cutoff, usable);

    for (const auto& box : boxes) {
        const auto source = centre(box.value) + glm::vec3{0, 0, -0.5};
        const auto receiver = centre(box.value) + glm::vec3{0, 0, 0.5};

        const auto scene =
                wayverb::core::geo::get_scene_data(box.value, surface);
        auto voxels_and_mesh = wayverb::waveguide::compute_voxels_and_mesh(
                cc, scene, source, sample_rate, environment.speed_of_sound);

        voxels_and_mesh.mesh.set_coefficients(
                wayverb::waveguide::to_flat_coefficients(absorption));

        const auto predicted_rt60 =
                max_element(wayverb::core::sabine_reverb_time(scene, 0.0f));

        std::cout << "predicted rt60 of room " << box.name << ": "
                  << predicted_rt60 << '\n';
        const size_t length = predicted_rt60 * 1.5 * sample_rate;

        util::aligned::vector<float> input(length, 0);
        input.front() = 1;

        const auto input_node =
                compute_index(voxels_and_mesh.mesh.get_descriptor(), source);
        const auto output_node =
                compute_index(voxels_and_mesh.mesh.get_descriptor(), receiver);

        auto prep = wayverb::waveguide::preprocessor::make_hard_source(
                input_node, begin(input), end(input));

        auto post = wayverb::core::callback_accumulator<
                wayverb::waveguide::postprocessor::node>{output_node};

        util::progress_bar pb;
        wayverb::waveguide::run(cc,
                                voxels_and_mesh.mesh,
                                prep,
                                [&](auto& a, const auto& b, auto c) {
                                    post(a, b, c);
                                    set_progress(pb, c, input.size());
                                },
                                true);

        auto results = post.get_output();

        const auto max_magnitude = wayverb::core::max_mag(results);
        const auto norm_factor = 1.0 / max_magnitude;

        wayverb::core::mul(results, norm_factor);

        write(util::build_string(box.name, ".wav").c_str(),
              results,
              sample_rate,
              audio_file::format::wav,
              audio_file::bit_depth::pcm24);
    }
}
