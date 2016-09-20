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

template <typename T>
void write(const std::string& name, T sig, double sample_rate) {
    const auto bit_depth{16};
    snd::write(build_string(name, ".wav"), {sig}, sample_rate, bit_depth);
    normalize(sig);
    snd::write(build_string("normalised.", name, ".wav"),
               {sig},
               sample_rate,
               bit_depth);
}

/// Attempts to replicate the physically modelled source tests from the paper
/// 'physical and numerical constraints in source modeling for finite
/// difference simulation of room acoustics' by Jonathan Sheaffer, Maarten van
/// Walstijn, and Brun Fazenda.

/// The actual test described in section V A from the sheaffer2014 paper.
void test_from_paper() {
    //  Set up all the generic features of the simulation.

    const geo::box box{glm::vec3{-3}, glm::vec3{3}};
    const auto sample_rate{16000.0};
    const auto speed_of_sound{340.0};

    const compute_context cc{};

    const glm::vec3 receiver{0};
    const auto radius{1.5};
    const glm::vec3 source{
            std::sin(M_PI / 4) * radius, 0, std::cos(M_PI / 4) * radius};

    auto voxels_and_mesh{
            waveguide::compute_voxels_and_mesh(cc,
                                               geo::get_scene_data(box),
                                               receiver,
                                               sample_rate,
                                               speed_of_sound)};

    auto& mesh{std::get<1>(voxels_and_mesh)};
    mesh.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(0.005991, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{compute_index(mesh.get_descriptor(), receiver)};

    //  Now we get to do the interesting new bit:
    //  Set up a physically modelled source signal.

    const auto length{1 << 12};
    auto pulse_shaping_filter{waveguide::maxflat(0.075, 16, 250e-6, length)};
    filter::biquad mechanical_filter{waveguide::mech_sphere(
            0.025, 100 / sample_rate, 0.7, 1 / sample_rate)};
    run_one_pass(mechanical_filter,
                 pulse_shaping_filter.signal.begin(),
                 pulse_shaping_filter.signal.end());
    const auto one_over_two_T{sample_rate / 2};
    filter::biquad injection_filter{{one_over_two_T, 0, -one_over_two_T, 0, 0}};
    run_one_pass(injection_filter,
                 pulse_shaping_filter.signal.begin(),
                 pulse_shaping_filter.signal.end());
    const auto input_signal{pulse_shaping_filter};

    //  Run the simulation.
    auto prep{waveguide::preprocessor::make_single_soft_source(
            input_node,
            input_signal.signal.begin(),
            input_signal.signal.end())};

    waveguide::postprocessor::output_accumulator<
            waveguide::postprocessor::node_state>
            postprocessor{output_node};

    progress_bar pb{std::cerr, input_signal.signal.size()};
    waveguide::run(cc,
                   mesh,
                   prep,
                   [&](auto& a, const auto& b, auto c) {
                       postprocessor(a, b, c);
                       pb += 1;
                   },
                   true);

    write("test_from_paper", postprocessor.get_output(), sample_rate);
}

auto get_mass_test_signals(double sample_rate) {
    return map_to_vector(
            aligned::vector<double>{0.025, 0.05, 0.1, 0.2, 0.4, 0.8},
            [=](auto i) {
                return waveguide::design_pcs_source(
                        1 << 15, sample_rate, i, 100, 1);
            });
}

auto get_cutoff_test_signals(double sample_rate) {
    return map_to_vector(aligned::vector<double>{20, 40, 60, 80, 100, 120},
                         [=](auto i) {
                             return waveguide::design_pcs_source(
                                     1 << 15, sample_rate, 0.01, i, 1);
                         });
}

void other_tests() {
    //  Set up all the generic features of the simulation.

    const geo::box box{glm::vec3{-3}, glm::vec3{3}};
    const auto sample_rate{4000.0};
    const auto speed_of_sound{340.0};

    const compute_context cc{};

    const glm::vec3 receiver{0};
    const auto radius{1.5};
    const glm::vec3 source{
            std::sin(M_PI / 4) * radius, 0, std::cos(M_PI / 4) * radius};

    auto voxels_and_mesh{
            waveguide::compute_voxels_and_mesh(cc,
                                               geo::get_scene_data(box),
                                               receiver,
                                               sample_rate,
                                               speed_of_sound)};

    auto& mesh{std::get<1>(voxels_and_mesh)};
    mesh.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(0.005991, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{compute_index(mesh.get_descriptor(), receiver)};

    //  Now we get to do the interesting new bit:
    //  Set up a physically modelled source signal.

    const auto run_tests{[&](auto name, auto signals) {
        auto count{0};
        for (const auto& input_signal : signals) {
            write(build_string(name, "_test_input_", count),
                  input_signal.signal,
                  sample_rate);

            //  Run the simulation.
            auto prep{waveguide::preprocessor::make_single_soft_source(
                    input_node,
                    input_signal.signal.begin(),
                    input_signal.signal.end())};

            waveguide::postprocessor::output_accumulator<
                    waveguide::postprocessor::node_state>
                    postprocessor{output_node};

            progress_bar pb{std::cerr, input_signal.signal.size()};
            waveguide::run(cc,
                           mesh,
                           prep,
                           [&](auto& a, const auto& b, auto c) {
                               postprocessor(a, b, c);
                               pb += 1;
                           },
                           true);

            write(build_string(name, "_test_output_", count),
                  postprocessor.get_output(),
                  sample_rate);

            count += 1;
        }
    }};

    // run_tests("mass", get_mass_test_signals(sample_rate));
    run_tests("cutoff", get_cutoff_test_signals(sample_rate));
}

int main() {
    other_tests();
    return EXIT_SUCCESS;
}
