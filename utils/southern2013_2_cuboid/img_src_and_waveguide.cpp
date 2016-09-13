#include "img_src_and_waveguide.h"

#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "waveguide/calibration.h"
#include "waveguide/default_kernel.h"
#include "waveguide/make_transparent.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/azimuth_elevation.h"
#include "common/frequency_domain_filter.h"
#include "common/progress_bar.h"
#include "common/schroeder.h"
#include "common/write_audio_file.h"

#include <iostream>

template <typename It>
float estimate_rt60(It begin, It end) {
    const auto measured_rt20{rt20(begin, end)};
    const auto measured_rt30{rt30(begin, end)};
    return std::get<0>(std::min(
            std::make_tuple(measured_rt20.samples * 3, measured_rt20.r),
            std::make_tuple(measured_rt30.samples * 2, measured_rt30.r),
            [](auto a, auto b) { return std::get<1>(a) < std::get<1>(b); }));
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

    //  Run raytracer.

    const auto directions{get_random_directions(100000)};

    auto impulses{raytracer::image_source::run<
            raytracer::image_source::comparison_calculator>(
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
            mixdown(raytracer::convert_to_histogram(
                    impulses.begin(), impulses.end(), sample_rate, 20)),
            [=](auto i) {
                return intensity_to_pressure(i, acoustic_impedance_);
            })};
    {
        static auto count{0};
        snd::write(build_string("raw_img_src_", count++, ".wav"),
                   {img_src_results},
                   sample_rate,
                   16);
    }

    const auto raytracer_rt60{
            estimate_rt60(img_src_results.begin(), img_src_results.end()) /
            sample_rate};
    std::cout << "raytracer rt60: " << raytracer_rt60 << '\n';

    //  Run waveguide.

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{
            compute_index(mesh.get_descriptor(), receiver.position)};

    const auto normalised_max{0.196};
    const waveguide::default_kernel kernel{waveguide_sample_rate_,
                                           normalised_max};
#if 0
    auto input_signal{kernel.kernel};
#else
    auto input_signal{waveguide::make_transparent(aligned::vector<float>{1})};
#endif

    input_signal.resize(img_src_results.size() * waveguide_sample_rate_ /
                                sample_rate +
                        kernel.correction_offset_in_samples);

    progress_bar pb{std::cout, input_signal.size()};
    const auto waveguide_results{waveguide::run(compute_context_,
                                                mesh,
                                                input_node,
                                                input_signal,
                                                output_node,
                                                speed_of_sound_,
                                                acoustic_impedance_,
                                                [&](auto i) { pb += 1; })};

    const float calibration_factor = waveguide::rectilinear_calibration_factor(
            waveguide_sample_rate_, speed_of_sound_);
    auto corrected_waveguide{waveguide::adjust_sampling_rate(
            map_to_vector(
                    waveguide_results,
                    [=](auto i) { return i.pressure * calibration_factor; }),
            waveguide_sample_rate_,
            sample_rate)};
    corrected_waveguide.erase(
            corrected_waveguide.begin(),
            corrected_waveguide.begin() + kernel.correction_offset_in_samples);

    {
        static auto count{0};
        snd::write(build_string("raw_waveguide_", count++, ".wav"),
                   {corrected_waveguide},
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
        const auto normalised_cutoff{0.15};
        const auto normalised_width{normalised_max - normalised_cutoff};

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
            snd::write(build_string("filtered_raytracer_", count++, ".wav"),
                       {img_src_results},
                       sample_rate,
                       16);
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
            snd::write(build_string("filtered_waveguide_", count++, ".wav"),
                       {corrected_waveguide},
                       sample_rate,
                       16);
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
