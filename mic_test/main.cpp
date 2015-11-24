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

using namespace std;
using namespace rapidjson;

MeshBoundary get_mesh_boundary(const SceneData& sd) {
    vector<Vec3f> v(sd.vertices.size());
    transform(sd.vertices.begin(),
              sd.vertices.end(),
              v.begin(),
              [](auto i) { return convert(i); });
    return MeshBoundary(sd.triangles, v);
}

int main(int argc, char** argv) {
    Logger::restart();
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 5) {
        Logger::log_err(
            "expecting a config file, an input model, an input material file, "
            "and an output filename");

        Logger::log_err("actually found: ");
        for (auto i = 0u; i != argc; ++i) {
            Logger::log_err("arg ", i, ": ", argv[i]);
        }

        return EXIT_FAILURE;
    }

    string config_file = argv[1];
    string model_file = argv[2];
    string material_file = argv[3];
    string output_file = argv[4];

    auto output_sr = 44100;
    auto bit_depth = 16;

    unsigned long format, depth;

    try {
        format = get_file_format(output_file);
        depth = get_file_depth(bit_depth);
    } catch (const runtime_error& e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    }

    //  global params
    auto speed_of_sound = 340.0;

    auto max_freq = 44100;
    auto filter_freq = max_freq * 0.5;
    auto sr = max_freq * 4;
    auto divisions = (speed_of_sound * sqrt(3)) / sr;

    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);

    cl_float3 source{{0, 2, 0}};
    cl_float3 mic{{0, 2, 5}};

    try {
        SceneData scene_data(model_file, material_file);

        auto boundary = get_mesh_boundary(scene_data);
        auto waveguide_program =
            get_program<TetrahedralProgram>(context, device);
        TetrahedralWaveguide waveguide(
            waveguide_program, queue, boundary, divisions);
        auto mic_index = waveguide.get_index_for_coordinate(convert(mic));
        auto source_index = waveguide.get_index_for_coordinate(convert(source));

        auto corrected_source =
            waveguide.get_coordinate_for_index(source_index);

        auto steps = 1 << 8;

        auto w_results =
            waveguide.run_basic(corrected_source, mic_index, steps, sr);

        Microphone microphone(Vec3f(0, 0, 1), 0.5);
        auto w_pressures = microphone.process(w_results);

        normalize(w_pressures);

        vector<float> out_signal(output_sr * w_results.size() / sr);

        SRC_DATA sample_rate_info{w_pressures.data(),
                                  out_signal.data(),
                                  long(w_results.size()),
                                  long(out_signal.size()),
                                  0,
                                  0,
                                  0,
                                  output_sr / double(sr)};

        src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);

        write_sndfile(output_file + ".waveguide.full.wav",
                      {out_signal},
                      output_sr,
                      depth,
                      format);

        LinkwitzRiley lopass;
        lopass.setParams(1, filter_freq, output_sr);
        lopass.filter(out_signal);

        normalize(out_signal);

        vector<vector<float>> waveguide_results = {out_signal};

        write_sndfile(output_file + ".waveguide.lopass.wav",
                      waveguide_results,
                      output_sr,
                      depth,
                      format);

    } catch (const cl::Error& e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const runtime_error& e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        Logger::log_err("unknown error");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
