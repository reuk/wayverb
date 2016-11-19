#include "waveguide/fitted_boundary.h"
#include "waveguide/mesh.h"
#include "waveguide/pcs.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/waveguide.h"

#include "core/callback_accumulator.h"
#include "core/dsp_vector_ops.h"

#include "utilities/map_to_vector.h"
#include "utilities/progress_bar.h"

#include "audio_file/audio_file.h"

#include <iostream>

template <typename T, typename Alloc>
void write(const std::string& name,
           std::vector<T, Alloc> sig,
           double sample_rate) {
    const auto bit_depth = audio_file::bit_depth::pcm16;
    write(util::build_string(name, ".wav").c_str(),
          sig,
          sample_rate,
          audio_file::format::wav,
          bit_depth);
    wayverb::core::normalize(sig);
    write(util::build_string("normalised.", name, ".wav").c_str(),
          sig,
          sample_rate,
          audio_file::format::wav,
          bit_depth);
}

/// Attempts to replicate the physically modelled source tests from the paper
/// 'physical and numerical constraints in source modeling for finite
/// difference simulation of room acoustics' by Jonathan Sheaffer, Maarten van
/// Walstijn, and Brun Fazenda.

/// The actual test described in section V A from the sheaffer2014 paper.
void test_from_paper() {
    //  Set up all the generic features of the simulation.

    const wayverb::core::geo::box box{glm::vec3{-3}, glm::vec3{3}};
    const auto sample_rate = 16000.0;
    const auto speed_of_sound = 340.0;

    const wayverb::core::compute_context cc{};

    const glm::vec3 receiver{0};
    const auto radius = 1.5;
    const glm::vec3 source{
            std::sin(M_PI / 4) * radius, 0, std::cos(M_PI / 4) * radius};

    auto voxels_and_mesh = wayverb::waveguide::compute_voxels_and_mesh(
            cc,
            wayverb::core::geo::get_scene_data(
                    box,
                    wayverb::core::make_surface<
                            wayverb::core::simulation_bands>(0, 0)),
            receiver,
            sample_rate,
            speed_of_sound);

    {
        constexpr auto absorption = 0.005991;
        voxels_and_mesh.mesh.set_coefficients(
                wayverb::waveguide::to_flat_coefficients(absorption));
    }

    const auto input_node =
            compute_index(voxels_and_mesh.mesh.get_descriptor(), source);
    const auto output_node =
            compute_index(voxels_and_mesh.mesh.get_descriptor(), receiver);

    //  Now we get to do the interesting new bit:
    //  Set up a physically modelled source signal.

    const auto length = 1 << 12;
    auto pulse_shaping_filter =
            wayverb::waveguide::maxflat(0.075, 16, 250e-6, length);
    wayverb::core::filter::biquad mechanical_filter{
            wayverb::waveguide::mech_sphere(
                    0.025, 100 / sample_rate, 0.7, 1 / sample_rate)};
    run_one_pass(mechanical_filter,
                 pulse_shaping_filter.signal.begin(),
                 pulse_shaping_filter.signal.end());
    const auto one_over_two_T = sample_rate / 2;
    wayverb::core::filter::biquad injection_filter{
            {one_over_two_T, 0, -one_over_two_T, 0, 0}};
    run_one_pass(injection_filter,
                 pulse_shaping_filter.signal.begin(),
                 pulse_shaping_filter.signal.end());
    const auto input_signal = pulse_shaping_filter;

    //  Run the simulation.
    auto prep = wayverb::waveguide::preprocessor::make_soft_source(
            input_node, input_signal.signal.begin(), input_signal.signal.end());

    wayverb::core::callback_accumulator<wayverb::waveguide::postprocessor::node>
            postprocessor{output_node};

    util::progress_bar pb;
    wayverb::waveguide::run(cc,
                            voxels_and_mesh.mesh,
                            prep,
                            [&](auto& a, const auto& b, auto c) {
                                postprocessor(a, b, c);
                                set_progress(pb, c, input_signal.signal.size());
                            },
                            true);

    write("test_from_paper", postprocessor.get_output(), sample_rate);
}

auto get_mass_test_signals(double acoustic_impedance,
                           double speed_of_sound,
                           double sample_rate) {
    const util::aligned::vector<double> sig{0.025, 0.05, 0.1, 0.2, 0.4, 0.8};
    return util::map_to_vector(begin(sig), end(sig), [=](auto i) {
        return wayverb::waveguide::design_pcs_source(1 << 15,
                                                     acoustic_impedance,
                                                     speed_of_sound,
                                                     sample_rate,
                                                     0.05,
                                                     i,
                                                     100,
                                                     1);
    });
}

auto get_cutoff_test_signals(double acoustic_impedance,
                             double speed_of_sound,
                             double sample_rate) {
    const util::aligned::vector<double> sig{20, 40, 60, 80, 100, 120};
    return util::map_to_vector(begin(sig), end(sig), [=](auto i) {
        return wayverb::waveguide::design_pcs_source(1 << 15,
                                                     acoustic_impedance,
                                                     speed_of_sound,
                                                     sample_rate,
                                                     0.05,
                                                     0.01,
                                                     i,
                                                     1);
    });
}

void other_tests() {
    //  Set up all the generic features of the simulation.

    const wayverb::core::geo::box box{glm::vec3{-3}, glm::vec3{3}};
    const auto sample_rate = 4000.0;
    const auto acoustic_impedance = 400.0;
    const auto speed_of_sound = 340.0;

    const wayverb::core::compute_context cc{};

    const glm::vec3 receiver{0};
    const auto radius = 1.5;
    const glm::vec3 source{
            std::sin(M_PI / 4) * radius, 0, std::cos(M_PI / 4) * radius};

    auto voxels_and_mesh = wayverb::waveguide::compute_voxels_and_mesh(
            cc,
            wayverb::core::geo::get_scene_data(
                    box,
                    wayverb::core::make_surface<
                            wayverb::core::simulation_bands>(0, 0)),
            receiver,
            sample_rate,
            speed_of_sound);

    voxels_and_mesh.mesh.set_coefficients(
            wayverb::waveguide::to_flat_coefficients(0.005991));

    const auto input_node =
            compute_index(voxels_and_mesh.mesh.get_descriptor(), source);
    const auto output_node =
            compute_index(voxels_and_mesh.mesh.get_descriptor(), receiver);

    //  Now we get to do the interesting new bit:
    //  Set up a physically modelled source signal.

    const auto run_tests = [&](auto name, auto signals) {
        auto count = 0;
        for (const auto& input_signal : signals) {
            write(util::build_string(name, "_test_input_", count),
                  input_signal.signal,
                  sample_rate);

            //  Run the simulation.
            auto prep = wayverb::waveguide::preprocessor::make_soft_source(
                    input_node,
                    input_signal.signal.begin(),
                    input_signal.signal.end());

            wayverb::core::callback_accumulator<
                    wayverb::waveguide::postprocessor::node>
                    postprocessor{output_node};

            util::progress_bar pb;
            wayverb::waveguide::run(
                    cc,
                    voxels_and_mesh.mesh,
                    prep,
                    [&](auto& a, const auto& b, auto c) {
                        postprocessor(a, b, c);
                        set_progress(pb, c, input_signal.signal.size());
                    },
                    true);

            write(util::build_string(name, "_test_output_", count),
                  postprocessor.get_output(),
                  sample_rate);

            count += 1;
        }
    };

    // run_tests("mass", get_mass_test_signals(sample_rate));
    run_tests("cutoff",
              get_cutoff_test_signals(
                      acoustic_impedance, speed_of_sound, sample_rate));
}

int main() {
    other_tests();
    return EXIT_SUCCESS;
}
