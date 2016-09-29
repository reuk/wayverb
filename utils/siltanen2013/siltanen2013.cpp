#include "waveguide/mesh.h"
#include "waveguide/pcs.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "raytracer/image_source/exact.h"
#include "raytracer/image_source/postprocessors.h"

#include "common/callback_accumulator.h"
#include "common/dsp_vector_ops.h"
#include "common/map_to_vector.h"
#include "common/progress_bar.h"
#include "common/sinc.h"
#include "common/write_audio_file.h"

#include <iostream>

template <typename T>
void write(const std::string& name, T t, double sample_rate) {
    const auto do_it{[&](const auto& name) {
        snd::write(build_string(name, ".wav"), {t}, sample_rate, 16);
    }};

    do_it(name);
    normalize(t);
    do_it(build_string("normalized.", name));

    const auto window{right_hanning(t.size())};
    elementwise_multiply(t, window);
    do_it(build_string("windowed.", name));
}

int main() {

    //  constants ------------------------------------------------------------//
    const geo::box box{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    constexpr auto sample_rate{16000.0};
    constexpr auto speed_of_sound{340.0};
    constexpr auto acoustic_impedance{400.0};

    constexpr glm::vec3 source{2.09, 2.12, 2.12};
    constexpr glm::vec3 receiver{2.09, 3.08, 0.96};

    const float absorption = 1 - pow(0.99, 2);

    //  waveguide ------------------------------------------------------------//
    const compute_context cc{};
    auto voxels_and_mesh{waveguide::compute_voxels_and_mesh(
            cc,
            geo::get_scene_data(box, make_surface(0, 0)),
            source,
            sample_rate,
            speed_of_sound)};

    auto& mesh{std::get<1>(voxels_and_mesh)};
    mesh.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(absorption, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{compute_index(mesh.get_descriptor(), receiver)};

    aligned::vector<float> input_signal{1.0f};
    input_signal.resize(4.0 * sample_rate, 0.0f);

    auto prep{waveguide::preprocessor::make_hard_source(
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

    write("waveguide", post.get_output(), sample_rate);

    //  exact image source ---------------------------------------------------//

    //  Find exact reflection coefficient products.
    auto impulses{raytracer::image_source::find_impulses<
            raytracer::image_source::fast_pressure_calculator<cl_float1>>(
            box, source, receiver, cl_float1{{absorption}})};

    //  Correct for distance travelled.
    for (auto& imp : impulses) {
        imp.volume *= pressure_for_distance(imp.distance, acoustic_impedance);
    }

    //  Create audio file.
    auto histogram{raytracer::sinc_histogram(
            impulses.begin(), impulses.end(), speed_of_sound, sample_rate, 20)};

    //  Extract.
    const auto img_src{map_to_vector(histogram, [](auto i) { return i.s[0]; })};

    write("exact_img_src", img_src, sample_rate);
}
