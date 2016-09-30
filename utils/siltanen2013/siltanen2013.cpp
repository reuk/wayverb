#include "waveguide/calibration.h"
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
#include "common/pressure_intensity.h"
#include "common/progress_bar.h"
#include "common/reverb_time.h"
#include "common/sinc.h"
#include "common/write_audio_file.h"

#include <iostream>

void postprocess_outputs(aligned::vector<float> waveguide,
                         aligned::vector<float> raytracer,
                         double grid_spacing,
                         double sample_rate) {
    const auto mag{std::max(max_mag(waveguide), max_mag(raytracer))};
    for (auto& i : waveguide) {
        i /= mag;
    }
    for (auto& i : raytracer) {
        i /= mag;
    }

    snd::write("waveguide.normal.raw.wav", {waveguide}, sample_rate, 16);
    snd::write("raytracer.normal.raw.wav", {raytracer}, sample_rate, 16);

    const auto calibration_factor{
            waveguide::rectilinear_calibration_factor(grid_spacing)};
    std::cout << "calibration factor: " << calibration_factor << '\n';

    for (auto& i : waveguide) {
        i *= calibration_factor;
    }

    snd::write("waveguide.calibrated.wav", {waveguide}, sample_rate, 16);

    const auto window{[](auto& sig) {
        elementwise_multiply(sig, right_hanning(sig.size()));
    }};

    window(waveguide);
    window(raytracer);

    snd::write("waveguide.windowed.wav", {waveguide}, sample_rate, 16);
    snd::write("raytracer.windowed.wav", {raytracer}, sample_rate, 16);
}

int main() {
    //  constants ------------------------------------------------------------//
    const geo::box box{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    constexpr auto sample_rate{16000.0};
    constexpr auto speed_of_sound{340.0};
    constexpr auto acoustic_impedance{400.0f};

    constexpr glm::vec3 source{2.09, 2.12, 2.12};
    constexpr glm::vec3 receiver{2.09, 3.08, 0.96};

    const float absorption = 1 - pow(0.95, 2);

    //  waveguide ------------------------------------------------------------//
    const compute_context cc{};
    auto voxels_and_mesh{waveguide::compute_voxels_and_mesh(
            cc,
            geo::get_scene_data(box, make_surface(absorption, 0)),
            source,
            sample_rate,
            speed_of_sound)};

    const auto eyring{
            eyring_reverb_time(std::get<0>(voxels_and_mesh).get_scene_data(), 0)
                    .s[0]};
    std::cout << "expected reverb time: " << eyring << '\n';

    auto& mesh{std::get<1>(voxels_and_mesh)};
    mesh.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(absorption, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{compute_index(mesh.get_descriptor(), receiver)};

    aligned::vector<float> input_signal{1.0f};
    input_signal.resize(eyring * sample_rate, 0.0f);

    std::cout << "running " << input_signal.size() << " steps\n";

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

    auto waveguide_output{post.get_output()};

    {
        //  Filter waveguide output.
        const auto waveguide_max{0.16};
        const auto normalised_width{0.02};
        const auto normalised_cutoff{waveguide_max - (normalised_width / 2)};

        const auto low_cutoff{100 / sample_rate};

        fast_filter filter{waveguide_output.size() * 2};
        filter.filter(waveguide_output.begin(),
                      waveguide_output.end(),
                      waveguide_output.begin(),
                      [=](auto cplx, auto freq) {
                          return compute_lopass_magnitude(freq,
                                                          normalised_cutoff,
                                                          normalised_width,
                                                          0) *
                                 compute_hipass_magnitude(
                                         freq, low_cutoff, low_cutoff * 2, 0) *
                                 cplx;
                      });
    }

    //  exact image source ---------------------------------------------------//

    //  Find exact reflection coefficient products.
    auto impulses{raytracer::image_source::find_all_impulses<
            raytracer::image_source::fast_pressure_calculator<cl_float1>>(
            box, source, receiver, cl_float1{{absorption}}, speed_of_sound)};

    //  Correct for distance travelled.
    for (auto& imp : impulses) {
        imp.volume *= pressure_for_distance(imp.distance, acoustic_impedance);
    }

    //  Mix down to histogram.
    const auto make_iterator{[=](auto i) {
        return raytracer::make_histogram_iterator(std::move(i), speed_of_sound);
    }};
    auto histogram{raytracer::sinc_histogram(make_iterator(impulses.begin()),
                                             make_iterator(impulses.end()),
                                             sample_rate,
                                             20)};

    //  Extract.
    auto img_src{map_to_vector(histogram, [](auto i) { return i.s[0]; })};

    postprocess_outputs(std::move(waveguide_output),
                        std::move(img_src),
                        std::get<1>(voxels_and_mesh).get_descriptor().spacing,
                        sample_rate);

    //  TODO compare to 'fast' image source output
}
