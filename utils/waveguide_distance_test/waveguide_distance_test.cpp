#include "waveguide/mesh.h"
#include "waveguide/pcs.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/postprocessor/output_accumulator.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/dsp_vector_ops.h"
#include "common/map_to_vector.h"
#include "common/progress_bar.h"
#include "common/write_audio_file.h"

#include <iostream>

template <typename It>
auto rms(It begin, It end) {
    if (begin == end) {
        throw std::runtime_error{"can't find rms of empty range"};
    }

    using std::sqrt;
    return sqrt(std::accumulate(
            begin + 1, end, *begin, [](const auto& i, const auto& j) {
                return i + j * j;
            }));
}

int main() {
    const geo::box box{glm::vec3{0}, glm::vec3{1, 1, 12}};
    const auto sample_rate{5000.0};
    const auto speed_of_sound{340.0};

    const compute_context cc{};

    const glm::vec3 source{0.5, 0.5, 0.5};

    aligned::vector<glm::vec3> receivers;
    for (auto i{0ul}; i < dimensions(box).z; ++i) {
        receivers.emplace_back(source + glm::vec3{0, 0, i});
    }

    auto voxels_and_mesh{waveguide::compute_voxels_and_mesh(
            cc, geo::get_scene_data(box), source, sample_rate, speed_of_sound)};

    auto& mesh{std::get<1>(voxels_and_mesh)};
    // mesh.set_coefficients(
    //        {waveguide::to_flat_coefficients(make_surface(0.005991, 0))});
    mesh.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(1, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), source)};

    //  Set up receivers.

    auto output_holders{map_to_vector(receivers, [&](auto i) {
        const auto receiver_index{compute_index(mesh.get_descriptor(), i)};
        if (!waveguide::is_inside(mesh, receiver_index)) {
            throw std::runtime_error{"receiver is outside of mesh!"};
        }
        return waveguide::postprocessor::output_accumulator<
                waveguide::postprocessor::node>{receiver_index};
    })};

    //  Set up a source signal.

    const auto input_signal{
            waveguide::design_pcs_source(1 << 16, sample_rate, 0.01, 100, 1)};
    auto prep{waveguide::preprocessor::make_soft_source(
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

    auto outputs{map_to_vector(output_holders,
                               [](const auto& i) { return i.get_output(); })};
    normalize(outputs);
    const auto mag_values{
            map_to_vector(outputs, [](const auto& i) { return max_mag(i); })};
    for (auto mag : mag_values) {
        std::cout << "mag: " << mag << '\n';
    }

    //  We expect to see a 1 / r^2 relationship between distance and rms.
    const auto rms_values{map_to_vector(
            outputs, [](const auto& i) { return rms(i.begin(), i.end()); })};
    for (auto rms : rms_values) {
        std::cout << "rms: " << rms << '\n';
    }

    auto count{0};
    for (const auto& i : outputs) {
        //  Write out.
        snd::write(
                build_string("distance_", count, ".wav"), {i}, sample_rate, 16);
        count += 1;
    }

    return EXIT_SUCCESS;
}
