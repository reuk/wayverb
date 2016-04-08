//  project internal
#include "conversions.h"
#include "microphone.h"
#include "scene_data.h"
#include "test_flag.h"
#include "waveguide.h"
#include "waveguide_config.h"

#include "raytracer.h"

#include "cl_common.h"

//  dependency
#include "filters_common.h"
#include "sinc.h"
#include "write_audio_file.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "samplerate.h"
#include "sndfile.hh"

#include <gflags/gflags.h>

//  stdlib
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
    auto config = WaveguideConfig();
    config.get_filter_frequency() = 11025;
    config.get_oversample_ratio() = 1;

    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);

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

    Microphone microphone(Vec3f(0, 0, 1), directionality);
    cl_float3 mic{{0, 0, 0}};
    auto test_locations = 18;

    std::ofstream ofile(polar_string + ".energies.txt");

    try {
        CuboidBoundary boundary(Vec3f(-2.05, -2.5, -1.05),
                                Vec3f(2.05, 2.5, 1.05));
        auto waveguide_program =
            get_program<TetrahedralProgram>(context, device);
        TetrahedralWaveguide waveguide(waveguide_program,
                                       queue,
                                       boundary,
                                       config.get_divisions(),
                                       to_vec3f(mic));

        auto amp_factor = 4e3;

        for (auto i = 0u; i != test_locations; ++i) {
            float angle = i * M_PI * 2 / test_locations + M_PI;
            cl_float3 source{{std::cos(angle), 0, std::sin(angle)}};

            auto mic_index = waveguide.get_index_for_coordinate(to_vec3f(mic));

            auto steps = 200;

            ProgressBar pb(std::cout, steps);
            auto w_results =
                waveguide.run_gaussian(to_vec3f(source),
                                       mic_index,
                                       steps,
                                       config.get_waveguide_sample_rate(),
                                       [&pb] { pb += 1; });

            auto w_pressures = microphone.process(w_results);

            std::vector<float> out_signal(config.get_output_sample_rate() *
                                          w_results.size() /
                                          config.get_waveguide_sample_rate());

            SRC_DATA sample_rate_info{w_pressures.data(),
                                      out_signal.data(),
                                      long(w_results.size()),
                                      long(out_signal.size()),
                                      0,
                                      0,
                                      0,
                                      config.get_output_sample_rate() /
                                          config.get_waveguide_sample_rate()};

            src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);

            mul(out_signal, amp_factor);

            filter::LinkwitzRileyLopass lopass;
            lopass.setParams(config.get_filter_frequency(),
                             config.get_output_sample_rate());
            lopass.filter(out_signal);

            auto bands = 7;
            auto min_band = 80;

            auto print_energy = [&ofile](const auto& sig, auto band) {
                auto band_energy = proc::accumulate(
                    sig, 0.0, [](auto a, auto b) { return a + b * b; });

                auto max_val = proc::accumulate(sig, 0.0, [](auto a, auto b) {
                    return std::max(a, fabs(b));
                });

                ofile << " band: " << band << " energy: " << band_energy
                      << " max: " << max_val;
            };

            ofile << "iteration: " << i;

            for (auto i = 0; i != bands; ++i) {
                auto band = out_signal;

                filter::LinkwitzRileyBandpass bandpass;
                bandpass.setParams(pow(2, i) * min_band,
                                   pow(2, i + 1) * min_band,
                                   config.get_output_sample_rate());
                bandpass.filter(out_signal);

                print_energy(band, i);
            }

            ofile << std::endl;

            std::stringstream ss;
            ss << output_folder << "/" << i << ".waveguide.full.wav";

            auto output_file = ss.str();

            unsigned long format, depth;

            try {
                format = get_file_format(output_file);
                depth = get_file_depth(config.get_bit_depth());
            } catch (const std::runtime_error& e) {
                LOG(INFO) << "critical runtime error: " << e.what();
                return EXIT_FAILURE;
            }

            write_sndfile(output_file,
                          {out_signal},
                          config.get_output_sample_rate(),
                          depth,
                          format);
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
