#include "waveguide/attenuator/microphone.h"
#include "waveguide/config.h"
#include "waveguide/default_kernel.h"
#include "waveguide/mesh/boundary_adjust.h"
#include "waveguide/mesh/model.h"
#include "waveguide/waveguide.h"

#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/filters_common.h"
#include "common/kernel.h"
#include "common/progress_bar.h"
#include "common/sinc.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/write_audio_file.h"

#include "samplerate.h"
#include "sndfile.hh"

#include <gflags/gflags.h>

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
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 3) {
        std::cerr << "expecting an output folder and a polar pattern\nactually "
                     "found: ";
        for (auto i = 0u; i != argc; ++i) {
            std::cerr << "arg " << i << ": " << argv[i];
        }

        return EXIT_FAILURE;
    }

    std::string output_folder = argv[1];
    std::string polar_string  = argv[2];

    aligned::map<std::string, PolarPattern> polar_pattern_map = {
            {"omni", PolarPattern::omni},
            {"cardioid", PolarPattern::cardioid},
            {"bidirectional", PolarPattern::bidirectional},
    };

    PolarPattern polar_pattern = polar_pattern_map[polar_string];
    std::cout << "polar_pattern: " << polar_string << std::endl;

    //  global params
    const auto filter_frequency = 11025.0;
    const auto oversample_ratio = 1.0;
    const auto waveguide_sr = filter_frequency * oversample_ratio * (1 / 0.196);

    compute_context cc;

    auto directionality = 0.0;
    switch (polar_pattern) {
        case PolarPattern::omni: directionality          = 0; break;
        case PolarPattern::cardioid: directionality      = 0.5; break;
        case PolarPattern::bidirectional: directionality = 1; break;
    }

    std::cout << "directionality: " << directionality << std::endl;

    waveguide::attenuator::microphone microphone;
    glm::vec3 mic{0, 0, 0};
    const auto test_locations = 12;

    std::ofstream ofile(output_folder + "/" + polar_string + ".energies.txt");

    try {
        const auto s = 1.5;
        const geo::box box(glm::vec3(-s), glm::vec3(s));
        auto scene_data = geo::get_scene_data(box);
        const auto r    = 0.9f;
        scene_data.set_surfaces(surface{volume_type{{r, r, r, r, r, r, r, r}},
                                        volume_type{{r, r, r, r, r, r, r, r}}});

        constexpr auto speed_of_sound{340.0};

        const auto voxels_and_model{
                waveguide::mesh::compute_voxels_and_model(cc.get_context(),
                                                          cc.get_device(),
                                                          scene_data,
                                                          mic,
                                                          waveguide_sr,
                                                          speed_of_sound)};

        const auto& model{std::get<1>(voxels_and_model)};

        for (auto i = 0u; i != test_locations; ++i) {
            float angle = i * M_PI * 2 / test_locations + M_PI;

            const auto kernel_info = waveguide::default_kernel(waveguide_sr);
            auto kernel            = kernel_info.kernel;

            glm::vec3 source{std::sin(angle), 0, std::cos(angle)};
            const auto dist = glm::distance(source, mic);
            const auto time_between_source_receiver = dist / speed_of_sound;
            const size_t required_steps =
                    time_between_source_receiver * waveguide_sr;

            const auto receiver_index =
                    compute_index(model.get_descriptor(), mic);
            const auto source_index =
                    compute_index(model.get_descriptor(), source);

            // const auto steps = required_steps +
            // kernel_info.opaque_kernel_size;
            const auto steps = 2 * required_steps;
            kernel.resize(steps);

            std::cout << "running " << steps << " steps" << std::endl;

            progress_bar pb(std::cout, steps);
            const auto w_results = waveguide::run(cc.get_context(),
                                                  cc.get_device(),
                                                  model,
                                                  source_index,
                                                  kernel,
                                                  receiver_index,
                                                  speed_of_sound,
                                                  400,
                                                  [&](auto) { pb += 1; });

            auto out_signal = microphone.process(
                    w_results, glm::vec3(0, 0, 1), directionality);

            // const auto bands = 8;
            // const auto min_band = 80;
            // const auto max_band = filter_frequency;

            // const auto factor = pow((max_band / min_band), 1.0 / bands);

            const auto print_energy = [&ofile](const auto& sig, auto band) {
                const auto band_energy = std::sqrt(proc::accumulate(
                        sig, 0.0, [](auto a, auto b) { return a + b * b; }));

                const auto max_val =
                        proc::accumulate(sig, 0.0, [](auto a, auto b) {
                            return std::max(a, fabs(b));
                        });

                ofile << " band: " << band << " energy: " << band_energy
                      << " max: " << max_val;
            };

            ofile << "iteration: " << i;

            //            std::cout << "//  BANDS" << std::endl;

            /*
            for (auto i = 0; i != bands; ++i) {
                auto band = out_signal;

                auto get_band_edge = [min_band, factor](auto i) {
                    return min_band * std::pow(factor, i);
                };

                const auto lower = get_band_edge(i + 0);
                const auto upper = get_band_edge(i + 1);

                //                std::cout << i + 1 << " : " << lower << " - "
                //                << upper
                //                          << std::endl;

                filter::LinkwitzRileyBandpass bandpass;
                bandpass.set_params(lower, upper, waveguide_sr);
                bandpass.filter(band);

                print_energy(band, i);
            }
            */
            print_energy(out_signal, 0);

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
