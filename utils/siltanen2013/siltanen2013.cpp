#include "waveguide/attenuator.h"
#include "waveguide/calibration.h"
#include "waveguide/config.h"
#include "waveguide/mesh.h"
#include "waveguide/pcs.h"
#include "waveguide/postprocessor/directional_receiver.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "raytracer/image_source/exact.h"
#include "raytracer/image_source/postprocessors.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "common/callback_accumulator.h"
#include "common/dsp_vector_ops.h"
#include "common/map_to_vector.h"
#include "common/pressure_intensity.h"
#include "common/progress_bar.h"
#include "common/reverb_time.h"
#include "common/sinc.h"
#include "common/write_audio_file.h"

#include <iostream>

//  TODO test with waveguide + raytracer microphone modelling.
//
//  TODO test raytracer diffuse output to compenstate for fast img-src decay

template <typename It>
auto waveguide_filter(It begin, It end, float sample_rate) {
    //  Filter waveguide output.
    const auto waveguide_max{0.16};
    const auto normalised_width{0.02};
    const auto normalised_cutoff{waveguide_max - (normalised_width / 2)};

    fast_filter filter{static_cast<size_t>(std::distance(begin, end)) * 2};
    filter.filter(begin, end, begin, [=](auto cplx, auto freq) {
        const auto ret{cplx *
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
}

auto run_waveguide(const geo::box& box,
                   float absorption,
                   const glm::vec3& source,
                   const glm::vec3& receiver,
                   const microphone& microphone,
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

    callback_accumulator<waveguide::postprocessor::directional_receiver> post{
            mesh.get_descriptor(),
            sample_rate,
            acoustic_impedance / speed_of_sound,
            output_node};

    progress_bar pb{std::cerr, input_signal.size()};
    waveguide::run(cc,
                   mesh,
                   prep,
                   [&](auto& queue, const auto& buffer, auto step) {
                       post(queue, buffer, step);
                       pb += 1;
                   },
                   true);

    auto attenuated{waveguide::attenuate(
            microphone, post.get_output().begin(), post.get_output().end())};

    waveguide_filter(attenuated.begin(), attenuated.end(), sample_rate);

    return attenuated;
}

template <typename It>
auto postprocess_impulses(It begin,
                          It end,
                          const glm::vec3& receiver,
                          const microphone& microphone,
                          float speed_of_sound,
                          float acoustic_impedance,
                          float sample_rate) {
    //  Correct for directionality of the receiver.
    auto attenuated{raytracer::attenuate(microphone, receiver, begin, end)};

    //  Correct for distance travelled.
    for (auto & it : attenuated) {
        it.volume *= pressure_for_distance(it.distance, acoustic_impedance);
    }

    //  Mix down to histogram.
    const auto histogram{raytracer::sinc_histogram(attenuated.begin(),
                                                   attenuated.end(),
                                                   speed_of_sound,
                                                   sample_rate,
                                                   20)};

    //  Extract.
    return map_to_vector(histogram, [](auto i) { return i.s[0]; });
}

auto run_exact_img_src(const geo::box& box,
                       float absorption,
                       const glm::vec3& source,
                       const glm::vec3& receiver,
                       const microphone& microphone,
                       float speed_of_sound,
                       float acoustic_impedance,
                       float sample_rate) {
    //  Find exact reflection coefficient products.
    auto impulses{raytracer::image_source::find_all_impulses<
            raytracer::image_source::fast_pressure_calculator<cl_float1>>(
            box, source, receiver, cl_float1{{absorption}}, speed_of_sound)};

    return postprocess_impulses(impulses.begin(),
                                impulses.end(),
                                receiver,
                                microphone,
                                speed_of_sound,
                                acoustic_impedance,
                                sample_rate);
}

auto run_fast_img_src(const geo::box& box,
                      float absorption,
                      const glm::vec3& source,
                      const glm::vec3& receiver,
                      const microphone& microphone,
                      float speed_of_sound,
                      float acoustic_impedance,
                      float sample_rate) {
    const compute_context cc{};

    const auto voxelised{make_voxelised_scene_data(
            geo::get_scene_data(box, make_surface(absorption, 0)), 2, 0.1f)};

    const auto directions{get_random_directions(1 << 13)};
    auto impulses{raytracer::image_source::run<
            raytracer::image_source::fast_pressure_calculator<surface>>(
            directions.begin(),
            directions.end(),
            cc,
            voxelised,
            source,
            receiver)};

    if (const auto direct{raytracer::get_direct(source, receiver, voxelised)}) {
        impulses.emplace_back(*direct);
    }

    return postprocess_impulses(impulses.begin(),
                                impulses.end(),
                                receiver,
                                microphone,
                                speed_of_sound,
                                acoustic_impedance,
                                sample_rate);
}

int main() {
    //  constants ------------------------------------------------------------//
    const geo::box box{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    constexpr auto sample_rate{16000.0};
    constexpr auto speed_of_sound{340.0};
    constexpr auto acoustic_impedance{400.0f};

    constexpr glm::vec3 source{2.09, 2.12, 2.12};
    constexpr glm::vec3 receiver{2.09, 3.08, 0.96};

    constexpr auto reflectance{0.8};
    const auto absorption{1 - pow(reflectance, 2)};

    const microphone mic{glm::vec3{0, 0, 1}, 0.0};

    const auto grid_spacing{
            waveguide::config::grid_spacing(speed_of_sound, 1 / sample_rate)};

    //  simulations ----------------------------------------------------------//

    std::cout << "running waveguide\n";
    auto waveguide{run_waveguide(box,
                                 absorption,
                                 source,
                                 receiver,
                                 mic,
                                 speed_of_sound,
                                 acoustic_impedance,
                                 sample_rate)};

    std::cout << "running exact img src\n";
    auto exact_img_src{run_exact_img_src(box,
                                         absorption,
                                         source,
                                         receiver,
                                         mic,
                                         speed_of_sound,
                                         acoustic_impedance,
                                         sample_rate)};

    std::cout << "running fast img src\n";
    auto fast_img_src{run_fast_img_src(box,
                                       absorption,
                                       source,
                                       receiver,
                                       mic,
                                       speed_of_sound,
                                       acoustic_impedance,
                                       sample_rate)};

    //  postprocessing -------------------------------------------------------//

    const auto outputs = {&waveguide, &exact_img_src, &fast_img_src};

    //  Normalise all outputs against one another.
    const auto max_mags{
            map_to_vector(outputs, [](auto i) { return max_mag(*i); })};

    const auto mag{*std::max_element(max_mags.begin(), max_mags.end())};

    for (const auto& ptr : outputs) {
        for (auto& samp : *ptr) {
            samp /= mag;
        }
    }

    snd::write("waveguide.normal.raw.wav", {waveguide}, sample_rate, 16);
    snd::write(
            "exact_img_src.normal.raw.wav", {exact_img_src}, sample_rate, 16);
    snd::write("fast_img_src.normal.raw.wav", {fast_img_src}, sample_rate, 16);

    const auto calibration_factor{waveguide::rectilinear_calibration_factor(
            grid_spacing, acoustic_impedance)};
    std::cout << "calibration factor: " << calibration_factor << '\n';

    for (auto& i : waveguide) {
        i *= calibration_factor;
    }

    snd::write("waveguide.calibrated.wav", {waveguide}, sample_rate, 16);
}
