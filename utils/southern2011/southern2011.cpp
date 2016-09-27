#include "waveguide/mesh.h"
#include "waveguide/pcs.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/callback_accumulator.h"
#include "common/dsp_vector_ops.h"
#include "common/map_to_vector.h"
#include "common/progress_bar.h"
#include "common/write_audio_file.h"

#include <iostream>

int main() {
    const geo::box box{glm::vec3{0}, glm::vec3{25.48, 17.57, 17.13}};
    const auto sample_rate{10000.0};
    const auto speed_of_sound{340.0};

    const glm::vec3 source{10.32, 10.78, 4.49};
    const glm::vec3 receiver{0.32, 10.78, 4.49};

    const auto time_in_seconds{0.1};
    const auto time_in_steps{time_in_seconds * sample_rate};

    const compute_context cc{};
    auto voxels_and_mesh{waveguide::compute_voxels_and_mesh(
            cc,
            geo::get_scene_data(box, make_surface(0, 0)),
            source,
            sample_rate,
            speed_of_sound)};

    auto& mesh{std::get<1>(voxels_and_mesh)};
    mesh.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(1, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{compute_index(mesh.get_descriptor(), receiver)};

    //auto input_signal{
    //        waveguide::design_pcs_source(1 << 16, sample_rate, 0.01, 100, 1)
    //                .signal};
    aligned::vector<float> input_signal{1.0f};
    input_signal.resize(time_in_steps);

    auto prep{waveguide::preprocessor::make_soft_source(
            input_node, input_signal.begin(), input_signal.end())};

    callback_accumulator<waveguide::postprocessor::node> post{output_node};

    progress_bar pb{std::cerr, input_signal.size()};
    waveguide::run(cc,
                   mesh,
                   prep,
                   [&](auto& queue, const auto& buffer, auto step) {
                       post(queue, buffer, step);
                       pb += 1;
                   },
                   true);

    snd::write("waveguide.wav", {post.get_output()}, sample_rate, 16);
}
