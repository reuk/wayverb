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
#include "raytracer/image_source/run.h"

#include "common/callback_accumulator.h"
#include "common/dsp_vector_ops.h"
#include "common/map_to_vector.h"
#include "common/pressure_intensity.h"
#include "common/progress_bar.h"
#include "common/reverb_time.h"
#include "common/sinc.h"
#include "common/write_audio_file.h"

#include <iostream>

auto run_waveguide(const geo::box& box,
                   float absorption,
                   const glm::vec3& source,
                   const glm::vec3& receiver,
                   float speed_of_sound,
                   float acoustic_impedance,
                   float sample_rate) {

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

    //  Filter waveguide output.
    const auto waveguide_max{0.16};
    const auto normalised_width{0.02};
    const auto normalised_cutoff{waveguide_max - (normalised_width / 2)};

    fast_filter filter{waveguide_output.size() * 2};
    filter.filter(
            waveguide_output.begin(),
            waveguide_output.end(),
            waveguide_output.begin(),
            [=](auto cplx, auto freq) {
                const auto ret{
                        cplx *
                        compute_lopass_magnitude(
                                freq, normalised_cutoff, normalised_width, 0)};
                const auto hipass{false};
                if (hipass) {
                    const auto low_cutoff{100 / sample_rate};
                    return ret * compute_hipass_magnitude(
                                         freq, low_cutoff, low_cutoff * 2, 0);
                }
                return ret;
            });

    return waveguide_output;
}

auto run_exact_img_src(const geo::box& box,
                       float absorption,
                       const glm::vec3& source,
                       const glm::vec3& receiver,
                       float speed_of_sound,
                       float acoustic_impedance,
                       float sample_rate) {
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
    return map_to_vector(histogram, [](auto i) { return i.s[0]; });
}

auto run_fast_img_src(const geo::box& box,
                      float absorption,
                      const glm::vec3& source,
                      const glm::vec3& receiver,
                      float speed_of_sound,
                      float acoustic_impedance,
                      float sample_rate) {
    const compute_context cc{};

    const auto voxelised{make_voxelised_scene_data(
            geo::get_scene_data(box, make_surface(absorption, 0)), 2, 0.1f)};

    const auto directions{get_random_directions(1<<13)};
    auto impulses{raytracer::image_source::run<
            raytracer::image_source::fast_pressure_calculator<surface>>(
            directions.begin(),
            directions.end(),
            cc,
            voxelised,
            source,
            receiver)};

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
    return map_to_vector(histogram, [](auto i) { return i.s[0]; });
}

int main() {
    //  constants ------------------------------------------------------------//
    const geo::box box{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    constexpr auto sample_rate{16000.0};
    constexpr auto speed_of_sound{340.0};
    constexpr auto acoustic_impedance{400.0f};

    constexpr glm::vec3 source{2.09, 2.12, 2.12};
    constexpr glm::vec3 receiver{2.09, 3.08, 0.96};

    constexpr auto reflectance{0.9};
    const auto absorption{1 - pow(reflectance, 2)};

    const compute_context cc{};
    auto voxels_and_mesh{waveguide::compute_voxels_and_mesh(
            cc,
            geo::get_scene_data(box, make_surface(absorption, 0)),
            source,
            sample_rate,
            speed_of_sound)};

    //  simulations ----------------------------------------------------------//

    std::cout << "running waveguide\n";
    auto waveguide{run_waveguide(box,
                                 absorption,
                                 source,
                                 receiver,
                                 speed_of_sound,
                                 acoustic_impedance,
                                 sample_rate)};

    std::cout << "running exact img src\n";
    auto exact_img_src{run_exact_img_src(box,
                                         absorption,
                                         source,
                                         receiver,
                                         speed_of_sound,
                                         acoustic_impedance,
                                         sample_rate)};

    std::cout << "running fast img src\n";
    auto fast_img_src{run_fast_img_src(box,
                                       absorption,
                                       source,
                                       receiver,
                                       speed_of_sound,
                                       acoustic_impedance,
                                       sample_rate)};

    //  postprocessing -------------------------------------------------------//

    const auto mag{std::max(max_mag(waveguide), max_mag(exact_img_src))};
    for (auto& i : waveguide) {
        i /= mag;
    }
    for (auto& i : exact_img_src) {
        i /= mag;
    }

    snd::write("waveguide.normal.raw.wav", {waveguide}, sample_rate, 16);
    snd::write(
            "exact_img_src.normal.raw.wav", {exact_img_src}, sample_rate, 16);

    const auto calibration_factor{waveguide::rectilinear_calibration_factor(
            std::get<1>(voxels_and_mesh).get_descriptor().spacing,
            acoustic_impedance)};
    std::cout << "calibration factor: " << calibration_factor << '\n';

    for (auto& i : waveguide) {
        i *= calibration_factor;
    }

    snd::write("waveguide.calibrated.wav", {waveguide}, sample_rate, 16);
}
