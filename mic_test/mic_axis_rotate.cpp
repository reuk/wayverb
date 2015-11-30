//  project internal
#include "waveguide.h"
#include "scene_data.h"
#include "test_flag.h"
#include "conversions.h"
#include "microphone.h"

#include "rayverb.h"

#include "cl_common.h"

//  dependency
#include "logger.h"
#include "filters_common.h"
#include "sinc.h"
#include "write_audio_file.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "sndfile.hh"
#include "samplerate.h"

#include <gflags/gflags.h>

//  stdlib
#include <random>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>

enum class PolarPattern {
    omni,
    cardioid,
    bidirectional,
};

int main(int argc, char** argv) {
    Logger::restart();
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 2) {
        Logger::log_err("expecting an output folder");

        Logger::log_err("actually found: ");
        for (auto i = 0u; i != argc; ++i) {
            Logger::log_err("arg ", i, ": ", argv[i]);
        }

        return EXIT_FAILURE;
    }

    std::string output_folder = argv[1];
    auto output_sr = 44100;
    auto bit_depth = 16;

    unsigned long format, depth;

    //  global params
    auto speed_of_sound = 340.0;

    auto max_freq = 11025;
    auto filter_freq = max_freq * 1.0;
    auto sr = max_freq * 4;
    auto divisions = (speed_of_sound * sqrt(3)) / sr;

    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);

    cl_float3 source{{0, 0, 0}};
    cl_float3 mic{{0, -1.15, 0}};
    auto test_locations = 36;

    try {
        CuboidBoundary boundary(Vec3f(-2.05, -2.5, -1.05),
                                Vec3f(2.05, 2.5, 1.05));
        auto waveguide_program =
            get_program<TetrahedralProgram>(context, device);
        TetrahedralWaveguide waveguide(
            waveguide_program, queue, boundary, divisions);
        auto mic_index = waveguide.get_index_for_coordinate(convert(mic));
        auto source_index = waveguide.get_index_for_coordinate(convert(source));

        auto corrected_source =
            waveguide.get_coordinate_for_index(source_index);
        auto corrected_mic = waveguide.get_coordinate_for_index(mic_index);

        std::cout << "source pos: " << corrected_source << std::endl;
        std::cout << "mic pos: " << corrected_mic << std::endl;

        //       auto steps = 4410;
        auto steps = 200;

        auto w_results =
            waveguide.run_basic(corrected_source, mic_index, steps, sr);

        auto amp_factor = 4e3;

        for (std::string polar_string : {"omni", "cardioid", "bidirectional"}) {
            std::map<std::string, PolarPattern> polar_pattern_map = {
                {"omni", PolarPattern::omni},
                {"cardioid", PolarPattern::cardioid},
                {"bidirectional", PolarPattern::bidirectional},
            };

            PolarPattern polar_pattern = polar_pattern_map[polar_string];
            std::cout << "polar_pattern: " << polar_string << std::endl;

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

            std::ofstream ofile(polar_string + ".r.energies.txt");

            for (auto i = 0u; i != test_locations; ++i) {
                auto angle = i * M_PI * 2 / test_locations + M_PI;

                Microphone microphone(Vec3f(sin(angle), cos(angle), 0),
                                      directionality);

                auto w_pressures = microphone.process(w_results);

                std::vector<float> out_signal(output_sr * w_results.size() /
                                              sr);

                SRC_DATA sample_rate_info{w_pressures.data(),
                                          out_signal.data(),
                                          long(w_results.size()),
                                          long(out_signal.size()),
                                          0,
                                          0,
                                          0,
                                          output_sr / double(sr)};

                src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);

                mul(out_signal, amp_factor);

                auto bands = 7;
                auto min_band = 80;

                ofile << "iteration: " << i;

                for (auto i = 0; i != bands; ++i) {
                    auto band = out_signal;

                    LinkwitzRiley bandpass;
                    bandpass.setParams(pow(2, i) * min_band,
                                       pow(2, i + 1) * min_band,
                                       output_sr);
                    bandpass.filter(out_signal);

                    auto band_energy = std::accumulate(
                        out_signal.begin(),
                        out_signal.end(),
                        0.0,
                        [](auto a, auto b) { return a + b * b; });

                    auto max_val = std::accumulate(
                        out_signal.begin(),
                        out_signal.end(),
                        0.0,
                        [](auto a, auto b) { return std::max(a, fabs(b)); });

                    ofile << " band: " << i << " energy: " << band_energy
                          << " max: " << max_val;
                }

                ofile << std::endl;

                std::stringstream ss;
                ss << output_folder << "/" << i << ".waveguide.full.wav";

                auto output_file = ss.str();

                try {
                    format = get_file_format(output_file);
                    depth = get_file_depth(bit_depth);
                } catch (const std::runtime_error& e) {
                    Logger::log_err("critical runtime error: ", e.what());
                    return EXIT_FAILURE;
                }

                LinkwitzRiley lopass;
                lopass.setParams(1, filter_freq, output_sr);
                lopass.filter(out_signal);

                write_sndfile(
                    output_file, {out_signal}, output_sr, depth, format);
            }
        }
    } catch (const cl::Error& e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const std::runtime_error& e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        Logger::log_err("unknown error");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
