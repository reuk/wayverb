#include "waveguide/mesh.h"
#include "waveguide/pcs.h"
#include "waveguide/postprocessor/output_holder.h"
#include "waveguide/postprocessor/single_node.h"
#include "waveguide/preprocessor/single_soft_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/dsp_vector_ops.h"
#include "common/map_to_vector.h"
#include "common/progress_bar.h"
#include "common/write_audio_file.h"

#include <iostream>

int main() {
    const geo::box box{glm::vec3{0}, glm::vec3{1, 1, 12}};
    const auto sample_rate{5000.0};
    const auto speed_of_sound{340.0};

    const compute_context cc{};

    const glm::vec3 source{0.5, 0.5, 0.5};

    const auto receivers{map_to_vector(
            aligned::vector<double>{
                    1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5},
            [](auto i) {
                return glm::vec3{0.5, 0.5, i};
            })};

    auto voxels_and_mesh{waveguide::compute_voxels_and_mesh(
            cc, geo::get_scene_data(box), source, sample_rate, speed_of_sound)};

    auto& mesh{std::get<1>(voxels_and_mesh)};
    mesh.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(0.005991, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), source)};

    //  Set up receivers.

    auto output_holders{map_to_vector(receivers, [&](auto i) {
        const auto receiver_index{compute_index(mesh.get_descriptor(), i)};
        if (!waveguide::is_inside(mesh, receiver_index)) {
            throw std::runtime_error{"receiver is outside of mesh!"};
        }
        return waveguide::postprocessor::output_accumulator<
                waveguide::postprocessor::node_state>{receiver_index};
    })};

    //  Now we get to do the interesting new bit:
    //  Set up a physically modelled source signal.

    const auto input_signal{
            waveguide::design_pcs_source(1 << 16, sample_rate, 0.01, 100, 1)};
    auto prep{waveguide::preprocessor::make_single_soft_source(
            input_node,
            input_signal.signal.begin(),
            input_signal.signal.end())};

    //  Run the simulation.

    progress_bar pb{std::cerr, input_signal.signal.size()};
    waveguide::run(cc,
                   mesh,
                   prep,
                   [&](auto& a, const auto& b, auto c) {
                       for (auto& i : output_holders) {
                           i(a, b, c);
                       }
                       pb += 1;
                   },
                   true);

    auto count{0};
    for (const auto& i : output_holders) {
        snd::write(build_string("distance_", count, ".wav"),
                   {i.get_output()},
                   sample_rate,
                   16);
        count += 1;
    }

    return EXIT_SUCCESS;
}
