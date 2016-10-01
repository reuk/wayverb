#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocess.h"
#include "waveguide/postprocessor/directional_receiver.h"
#include "waveguide/preprocessor/gaussian.h"
#include "waveguide/waveguide.h"

#include "common/aligned/map.h"
#include "common/callback_accumulator.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/filters_common.h"
#include "common/kernel.h"
#include "common/progress_bar.h"
#include "common/sinc.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/write_audio_file.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

enum class PolarPattern {
    omni,
    cardioid,
    bidirectional,
};

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "expecting an output folder and a polar pattern\nactually "
                     "found: ";
        for (auto i{0u}; i != argc; ++i) {
            std::cerr << "arg " << i << ": " << argv[i];
        }

        return EXIT_FAILURE;
    }

    std::string output_folder{argv[1]};
    std::string polar_string{argv[2]};

    aligned::map<std::string, PolarPattern> polar_pattern_map{
            {"omni", PolarPattern::omni},
            {"cardioid", PolarPattern::cardioid},
            {"bidirectional", PolarPattern::bidirectional}};

    const auto polar_pattern{polar_pattern_map[polar_string]};
    std::cout << "polar_pattern: " << polar_string << std::endl;

    //  global params
    const auto filter_frequency{11025.0};
    const auto oversample_ratio{1.0};
    const auto waveguide_sr{filter_frequency * oversample_ratio * (1 / 0.196)};

    const compute_context cc{};

    const auto find_directionality{[](auto polar_pattern) {
        switch (polar_pattern) {
            case PolarPattern::omni: return 0.0f;
            case PolarPattern::cardioid: return 0.5f;
            case PolarPattern::bidirectional: return 1.0f;
        }
    }};

    const auto directionality{find_directionality(polar_pattern)};

    std::cout << "directionality: " << directionality << std::endl;

    const glm::vec3 mic{0, 0, 0};
    const auto test_locations{12};

    std::ofstream ofile{output_folder + "/" + polar_string + ".energies.txt"};

    try {
        const auto s{1.5f};
        const geo::box box{glm::vec3{-s}, glm::vec3{s}};
        const auto r{0.9f};
        const auto scene_data{geo::get_scene_data(box, make_surface(r, r))};

        constexpr auto speed_of_sound{340.0};
        constexpr auto acoustic_impedance{400.0};

        const auto voxels_and_model{waveguide::compute_voxels_and_mesh(
                cc, scene_data, mic, waveguide_sr, speed_of_sound)};

        const auto& model{std::get<1>(voxels_and_model)};

        for (auto i{0u}; i != test_locations; ++i) {
            const auto angle{i * M_PI * 2 / test_locations + M_PI};

            const glm::vec3 source{std::sin(angle), 0, std::cos(angle)};
            const auto dist{glm::distance(source, mic)};
            const auto time_between_source_receiver{dist / speed_of_sound};
            const size_t required_steps =
                    time_between_source_receiver * waveguide_sr;

            const auto receiver_index{
                    compute_index(model.get_descriptor(), mic)};

            const auto steps{2 * required_steps};

            //  hacıhabiboglu2010 the pulse had a variance of 4 spatial samples
            const auto variance{4 * model.get_descriptor().spacing};
            //  standard deviation is the sqrt of the variance
            const waveguide::preprocessor::gaussian generator{
                    model.get_descriptor(), source, std::sqrt(variance), steps};

            callback_accumulator<waveguide::postprocessor::directional_receiver>
                    postprocessor{model.get_descriptor(),
                                  waveguide_sr,
                                  acoustic_impedance / speed_of_sound,
                                  receiver_index};

            std::cout << "running " << steps << " steps" << std::endl;

            progress_bar pb(std::cout, steps);
            waveguide::run(cc,
                           model,
                           generator,
                           [&](auto& queue, const auto& buffer, auto step) {
                               postprocessor(queue, buffer, step);
                               pb += 1;
                           },
                           true);

            auto out_signal{waveguide::attenuate(
                    microphone{glm::vec3{0, 0, 1}, directionality},
                    postprocessor.get_output().begin(),
                    postprocessor.get_output().end())};

            const auto bands{8};
            const auto min_band{80};
            const auto max_band{filter_frequency};

            const auto factor{pow((max_band / min_band), 1.0 / bands)};

            const auto print_energy{[&ofile](const auto& sig, auto band) {
                const auto band_energy = std::sqrt(proc::accumulate(
                        sig, 0.0, [](auto a, auto b) { return a + b * b; }));

                const auto max_val =
                        proc::accumulate(sig, 0.0, [](auto a, auto b) {
                            return std::max(a, fabs(b));
                        });

                ofile << " band: " << band << " energy: " << band_energy
                      << " max: " << max_val;
            }};

            ofile << "iteration: " << i;

            for (auto i{0u}; i != bands; ++i) {
                auto band{out_signal};

                const auto get_band_edge{
                        [&](auto i) { return min_band * std::pow(factor, i); }};

                const auto lower{get_band_edge(i + 0)};
                const auto upper{get_band_edge(i + 1)};

                filter::linkwitz_riley_bandpass(
                        lower, upper, waveguide_sr, band.begin(), band.end());

                print_energy(band, i);
            }

            ofile << std::endl;

            snd::write(
                    build_string(output_folder, "/", i, ".waveguide.full.wav"),
                    {out_signal},
                    waveguide_sr,
                    16);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "critical runtime error: " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "unknown error\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
