#include "img_src_and_waveguide.h"

#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "waveguide/calibration.h"
#include "waveguide/config.h"
#include "waveguide/make_transparent.h"
#include "waveguide/pcs.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/azimuth_elevation.h"
#include "common/callback_accumulator.h"
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

template <typename It, typename T>
void run_fast_filter(It begin, It end, const T& callback) {
    const auto sig_size{std::distance(begin, end)};
    //  Set up buffer, twice the size of input signal.
    aligned::vector<float> input(sig_size * 2, 0.0f);
    //  Copy input signal halfway through input buffer.
    const auto input_begin{input.begin() + input.size() / 4};
    std::copy(begin, end, input_begin);
    //  Set up and run the filter.
    fast_filter filter{input.size()};
    filter.filter(input.begin(), input.end(), input.begin(), callback);
    //  Copy output to iterator.
    std::copy(input_begin, input_begin + sig_size, begin);
}

img_src_and_waveguide_test::img_src_and_waveguide_test(
        const generic_scene_data<cl_float3, surface>& sd,
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
        const surface& surf,
        const glm::vec3& source,
        const model::receiver_settings& receiver) {
    auto& voxels{std::get<0>(voxels_and_mesh_)};
    voxels.set_surfaces(surf);

    auto& mesh{std::get<1>(voxels_and_mesh_)};
    // mesh.set_coefficients(waveguide::to_filter_coefficients(
    //        voxels.get_scene_data().get_surfaces(), waveguide_sample_rate_));
    mesh.set_coefficients(waveguide::to_flat_coefficients(
            voxels.get_scene_data().get_surfaces()));

    const auto sample_rate{44100.0};

    //  Run raytracer.  ------------------------------------------------------//

    const auto directions{get_random_directions(10000)};

    auto impulses{raytracer::image_source::run<
            raytracer::image_source::fast_pressure_calculator<surface>>(
            directions.begin(),
            directions.end(),
            compute_context_,
            voxels,
            source,
            receiver.position)};

    if (const auto direct{
                raytracer::get_direct(source, receiver.position, voxels)}) {
        impulses.emplace_back(*direct);
    }

    for (auto& imp : impulses) {
        imp.volume *= pressure_for_distance(imp.distance, acoustic_impedance_);
    }

    const auto make_iterator{[=](auto i) {
        return raytracer::make_histogram_iterator(std::move(i),
                                                  speed_of_sound_);
    }};
    auto histogram{raytracer::dirac_histogram(make_iterator(impulses.begin()),
                                              make_iterator(impulses.end()),
                                              sample_rate,
                                              20)};

    //  TODO Filter properly.
    //  For now we just divide pressures by 8.
    for (auto& samp : histogram) {
        samp /= 8;
    }

    //  Mixdown.
    auto img_src_results{mixdown(histogram.begin(), histogram.end())};

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
    aligned::vector<float> input_signal{1.0f};
    input_signal.resize(img_src_results.size() * waveguide_sample_rate_ /
                        sample_rate);
    auto prep{waveguide::preprocessor::make_hard_source(
            input_node, input_signal.begin(), input_signal.end())};

    callback_accumulator<waveguide::postprocessor::node> postprocessor{
            output_node};

    //  Run the waveguide simulation.
    progress_bar pb{std::cerr, input_signal.size()};
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
            mesh.get_descriptor().spacing, acoustic_impedance_);

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
        run_fast_filter(img_src_results.begin(),
                        img_src_results.end(),
                        [=](auto cplx, auto freq) {
                            return cplx *
                                   static_cast<float>(compute_hipass_magnitude(
                                           freq, cutoff, width, 0));
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

        run_fast_filter(corrected_waveguide.begin(),
                        corrected_waveguide.end(),
                        [=](auto cplx, auto freq) {
                            return cplx *
                                   static_cast<float>(compute_lopass_magnitude(
                                           freq, cutoff, width, 0));
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
