#include "img_src_and_waveguide.h"

#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "waveguide/calibration.h"
#include "waveguide/config.h"
#include "waveguide/make_transparent.h"
#include "waveguide/pcs.h"
#include "waveguide/postprocessor/output_holder.h"
#include "waveguide/postprocessor/single_node.h"
#include "waveguide/preprocessor/single_soft_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/azimuth_elevation.h"
#include "common/dc_blocker.h"
#include "common/dsp_vector_ops.h"
#include "common/frequency_domain_filter.h"
#include "common/progress_bar.h"
#include "common/schroeder.h"
#include "common/write_audio_file.h"

#include <iostream>

template <typename It>
float estimate_rt60(It begin, It end) {
    return std::min(rt20(begin, end),
                    rt30(begin, end),
                    [](auto a, auto b) { return a.r < b.r; })
            .samples;
}

img_src_and_waveguide_test::img_src_and_waveguide_test(const scene_data& sd,
                                                       float speed_of_sound,
                                                       float acoustic_impedance)
        : voxels_and_mesh_{waveguide::compute_voxels_and_mesh(
                  compute_context_,
                  sd,
                  glm::vec3{0},
                  waveguide_sample_rate_,
                  speed_of_sound)}
        , speed_of_sound_{speed_of_sound}
        , acoustic_impedance_{acoustic_impedance} {}

audio img_src_and_waveguide_test::operator()(
        const surface& surface,
        const glm::vec3& source,
        const model::receiver_settings& receiver) {
    auto& voxels{std::get<0>(voxels_and_mesh_)};
    voxels.set_surfaces(surface);

    auto& mesh{std::get<1>(voxels_and_mesh_)};
    // mesh.set_coefficients(waveguide::to_filter_coefficients(
    //        voxels.get_scene_data().get_surfaces(), waveguide_sample_rate_));
    mesh.set_coefficients(waveguide::to_flat_coefficients(
            voxels.get_scene_data().get_surfaces()));

    const auto sample_rate{44100.0};

    //  Run raytracer.  ------------------------------------------------------//

    const auto directions{get_random_directions(1000)};

    auto impulses{raytracer::image_source::run<
            raytracer::image_source::fast_pressure_calculator>(
            directions.begin(),
            directions.end(),
            compute_context_,
            voxels,
            source,
            receiver.position,
            speed_of_sound_,
            acoustic_impedance_,
            sample_rate)};

    if (const auto direct{raytracer::get_direct_impulse(
                source, receiver.position, voxels, speed_of_sound_)}) {
        impulses.emplace_back(*direct);
    }

    auto img_src_results{map_to_vector(
            mixdown(raytracer::convert_to_histogram(impulses.begin(),
                                                    impulses.end(),
                                                    speed_of_sound_,
                                                    sample_rate,
                                                    20)),
            [=](auto i) {
                return intensity_to_pressure(i, acoustic_impedance_);
            })};
    {
        static auto count{0};
        auto copy{img_src_results};
        normalize(copy);
        snd::write(build_string("raw_img_src_", count++, ".wav"),
                   {copy},
                   sample_rate,
                   16);
    }

    for (auto i{1ul}, end{img_src_results.size()}; i < end; ++i) {
        const auto time{i / sample_rate};
        const auto distance{time * speed_of_sound_};
        img_src_results[i] *=
                pressure_for_distance(distance, acoustic_impedance_);
    }

    {
        static auto count{0};
        auto copy{img_src_results};
        normalize(copy);
        snd::write(build_string("raw_img_src_attenuated_", count++, ".wav"),
                   {copy},
                   sample_rate,
                   16);
    }

    const auto raytracer_rt60{
            estimate_rt60(img_src_results.begin(), img_src_results.end()) /
            sample_rate};
    std::cout << "raytracer rt60: " << raytracer_rt60 << '\n' << std::flush;

    //  Run waveguide.  ------------------------------------------------------//

    //  Find input and output nodes.
    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{
            compute_index(mesh.get_descriptor(), receiver.position)};

    //  Create input signal.
    const auto waveguide_steps{img_src_results.size() * waveguide_sample_rate_ /
                               sample_rate};
    const auto input_signal{waveguide::design_pcs_source(
            waveguide_steps, waveguide_sample_rate_, 0.01, 100, 1)};
    auto prep{waveguide::preprocessor::make_single_soft_source(
            input_node,
            input_signal.signal.begin(),
            input_signal.signal.end())};

    waveguide::postprocessor::output_accumulator<
            waveguide::postprocessor::node_state>
            postprocessor{output_node};

    //  Run the waveguide simulation.
    progress_bar pb{std::cerr, input_signal.signal.size()};
    waveguide::run(compute_context_,
                   mesh,
                   prep,
                   [&](auto& a, const auto& b, auto c) {
                       postprocessor(a, b, c);
                       pb += 1;
                   },
                   true);

    //  Compute required amplitude adjustment.
    const float calibration_factor = waveguide::rectilinear_calibration_factor(
            waveguide_sample_rate_, speed_of_sound_);

    //  Adjust magnitude.
    auto magnitude_adjusted{
            map_to_vector(postprocessor.get_output(),
                          [=](auto i) { return i * calibration_factor; })};

    {
        static auto count{0};
        auto copy{magnitude_adjusted};
        normalize(copy);
        snd::write(build_string("raw_waveguide_", count++, ".wav"),
                   {copy},
                   sample_rate,
                   16);
    }

    //  Remove initial samples from waveguide output so that it lines up with
    //  raytracer output.
//    magnitude_adjusted.erase(magnitude_adjusted.begin(),
//                             magnitude_adjusted.begin() + input_signal.offset);

    //  Convert sampling rate.
    auto corrected_waveguide{waveguide::adjust_sampling_rate(
            magnitude_adjusted, waveguide_sample_rate_, sample_rate)};

    {
        static auto count{0};
        auto copy{corrected_waveguide};
        normalize(copy);
        snd::write(build_string("corrected_waveguide_", count++, ".wav"),
                   {copy},
                   sample_rate,
                   16);
    }

    const auto waveguide_rt60{estimate_rt60(corrected_waveguide.begin(),
                                            corrected_waveguide.end()) /
                              waveguide_sample_rate_};
    std::cout << "waveguide rt60: " << waveguide_rt60 << '\n';

    //  Filter + postprocess

    const auto max_size{
            std::max(corrected_waveguide.size(), img_src_results.size())};
    {
        //  Do some crossover filtering
        const auto waveguide_max{0.16};
        const auto normalised_width{0.02};
        const auto normalised_cutoff{waveguide_max - (normalised_width / 2)};

        const auto adjust_frequency{[=](auto i) {
            return i * waveguide_sample_rate_ / sample_rate;
        }};

        const auto cutoff{adjust_frequency(normalised_cutoff)};
        const auto width{adjust_frequency(normalised_width)};

        img_src_results.resize(max_size);
        corrected_waveguide.resize(max_size);
        fast_filter filter{max_size * 2};
        filter.filter(img_src_results.begin(),
                      img_src_results.end(),
                      img_src_results.begin(),
                      [=](auto cplx, auto freq) {
                          return cplx * compute_hipass_magnitude(
                                                freq, cutoff, width, 0);
                      });

        {
            static auto count{0};
            auto copy{img_src_results};
            normalize(copy);
            snd::write(build_string("filtered_raytracer_", count++, ".wav"),
                       {copy},
                       sample_rate,
                       16);

            const auto rt60{estimate_rt60(copy.begin(), copy.end()) /
                            sample_rate};
            std::cout << "filtered raytracer rt60: " << rt60 << '\n';
        }

        filter.filter(corrected_waveguide.begin(),
                      corrected_waveguide.end(),
                      corrected_waveguide.begin(),
                      [=](auto cplx, auto freq) {
                          return cplx * compute_lopass_magnitude(
                                                freq, cutoff, width, 0);
                      });

        {
            static auto count{0};
            auto copy{corrected_waveguide};
            normalize(copy);
            snd::write(build_string("filtered_waveguide_", count++, ".wav"),
                       {copy},
                       sample_rate,
                       16);
            const auto rt60{estimate_rt60(copy.begin(), copy.end()) /
                            sample_rate};
            std::cout << "filtered waveguide rt60: " << rt60 << '\n';
        }
    }

    aligned::vector<float> output(max_size);

    for (auto i{0ul}, end{img_src_results.size()}; i != end; ++i) {
        output[i] += img_src_results[i];
    }

    for (auto i{0ul}, end{corrected_waveguide.size()}; i != end; ++i) {
        output[i] += corrected_waveguide[i];
    }

    return {std::move(output), sample_rate, "img_src_and_waveguide"};
}
