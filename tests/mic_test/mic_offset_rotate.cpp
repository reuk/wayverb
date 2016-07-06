#include "waveguide/config.h"
#include "waveguide/microphone.h"
#include "waveguide/rectangular_waveguide.h"

#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/kernel.h"
#include "common/scene_data.h"

#include "common/filters_common.h"
#include "common/sinc.h"
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
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 3) {
        LOG(INFO) << "expecting an output folder and a polar pattern";

        LOG(INFO) << "actually found: ";
        for (auto i = 0u; i != argc; ++i) {
            LOG(INFO) << "arg " << i << ": " << argv[i];
        }

        return EXIT_FAILURE;
    }

    std::string output_folder = argv[1];
    std::string polar_string = argv[2];

    std::map<std::string, PolarPattern> polar_pattern_map = {
            {"omni", PolarPattern::omni},
            {"cardioid", PolarPattern::cardioid},
            {"bidirectional", PolarPattern::bidirectional},
    };

    PolarPattern polar_pattern = polar_pattern_map[polar_string];
    std::cout << "polar_pattern: " << polar_string << std::endl;

    //  global params
    const auto filter_frequency = 11025.0;
    const auto oversample_ratio = 1.0;
    const auto waveguide_sr = filter_frequency * oversample_ratio * 4;

    ComputeContext compute_context;

    auto directionality = 0.0;
    switch (polar_pattern) {
        case PolarPattern::omni:
            directionality = 0;
            break;
        case PolarPattern::cardioid:
            directionality = 0.5;
            break;
        case PolarPattern::bidirectional:
            directionality = 1;
            break;
    }

    std::cout << "directionality: " << directionality << std::endl;

    Microphone microphone(glm::vec3(0, 0, 1), directionality);
    glm::vec3 mic{0, 0, 0};
    const auto test_locations = 18;

    std::ofstream ofile(output_folder + "/" + polar_string + ".energies.txt");

    try {
        CuboidBoundary boundary(glm::vec3(-2.05, -2.5, -1.05),
                                glm::vec3(2.05, 2.5, 1.05));
        const RectangularProgram waveguide_program(compute_context.context,
                                                   compute_context.device);
        RectangularWaveguide<BufferType::cl> waveguide(
                waveguide_program,
                MeshBoundary(boundary.get_scene_data()),
                mic,
                waveguide_sr);

        const auto amp_factor = 4e3;

        for (auto i = 0u; i != test_locations; ++i) {
            float angle = i * M_PI * 2 / test_locations + M_PI;

            const auto mic_index = waveguide.get_index_for_coordinate(mic);

            const auto steps = 200;

            std::atomic_bool keep_going{true};
            ProgressBar pb(std::cout, steps);
            const auto w_results = waveguide.init_and_run(
                    glm::vec3{std::sin(angle), 0, std::cos(angle)},
                    kernels::sin_modulated_gaussian_kernel(waveguide_sr),
                    mic_index,
                    steps,
                    keep_going,
                    [&pb] { pb += 1; });

            const auto out_sr = 44100.0;

            auto out_signal = adjust_sampling_rate(
                    microphone.process(w_results), waveguide_sr, out_sr);

            mul(out_signal, amp_factor);

            filter::LinkwitzRileyLopass lopass;
            lopass.set_params(filter_frequency, out_sr);
            lopass.filter(out_signal);

            const auto bands = 7;
            const auto min_band = 80;

            const auto print_energy = [&ofile](const auto& sig, auto band) {
                const auto band_energy = proc::accumulate(
                        sig, 0.0, [](auto a, auto b) { return a + b * b; });

                const auto max_val =
                        proc::accumulate(sig, 0.0, [](auto a, auto b) {
                            return std::max(a, fabs(b));
                        });

                ofile << " band: " << band << " energy: " << band_energy
                      << " max: " << max_val;
            };

            ofile << "iteration: " << i;

            for (auto i = 0; i != bands; ++i) {
                const auto band = out_signal;

                filter::LinkwitzRileyBandpass bandpass;
                bandpass.set_params(
                        pow(2, i) * min_band, pow(2, i + 1) * min_band, out_sr);
                bandpass.filter(out_signal);

                print_energy(band, i);
            }

            ofile << std::endl;

            snd::write(
                    build_string(output_folder, "/", i, ".waveguide.full.wav"),
                    {out_signal},
                    out_sr,
                    16);
        }

    } catch (const cl::Error& e) {
        LOG(INFO) << "critical cl error: " << e.what();
        return EXIT_FAILURE;
    } catch (const std::runtime_error& e) {
        LOG(INFO) << "critical runtime error: " << e.what();
        return EXIT_FAILURE;
    } catch (...) {
        LOG(INFO) << "unknown error";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
