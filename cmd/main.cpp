//  project internal
#include "waveguide.h"
#include "scene_data.h"
#include "test_flag.h"
#include "conversions.h"

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
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>

using namespace std;

MeshBoundary get_mesh_boundary(const SceneData & sd) {
    vector<Vec3f> v(sd.vertices.size());
    transform(sd.vertices.begin(),
              sd.vertices.end(),
              v.begin(),
              [](auto i) { return convert(i); });
    return MeshBoundary(sd.triangles, v);
}

vector<float> exponential_decay_envelope(int steps, float attenuation_factor) {
    vector<float> ret(steps);
    auto amp = 1.0f;
    generate(begin(ret),
             end(ret),
             [&amp, attenuation_factor] {
                 auto t = amp;
                 amp *= attenuation_factor;
                 return t;
             });
    return ret;
}

bool all_zero(const vector<float> & t) {
    auto ret = true;
    for (auto i = 0u; i != t.size(); ++i) {
        if (t[i] != 0) {
            Logger::log("non-zero at element: ", i, ", value: ", t[i]);
            ret = false;
        }
    }
    return ret;
}

int main(int argc, char ** argv) {
    Logger::restart();
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 4) {
        Logger::log_err(
            "expecting an input model, an input material file, and an output "
            "filename");
        return EXIT_FAILURE;
    }

    string model_file = argv[1];
    string material_file = argv[2];
    string output_file = argv[3];

    auto speed_of_sound = 340.0;
    auto attenuation_factor = 0.999;

    auto max_freq = 500;
    auto sr = max_freq * 4;
    auto divisions = (speed_of_sound * sqrt(3)) / sr;

    auto output_sr = 44100;

    auto steps = 1 << 13;

    auto bitDepth = 16;

    unsigned long format, depth;

    try {
        format = get_file_format(output_file);
        depth = get_file_depth(bitDepth);
    } catch (const runtime_error & e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    }

    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);

    try {
        vector<cl_float> results;

        auto boundary = get_mesh_boundary(SceneData(model_file, material_file));

        auto program = get_program<TetrahedralProgram>(context, device);
        IterativeTetrahedralWaveguide waveguide(
            program, queue, boundary, divisions);
        results = waveguide.run(20000, 20000, steps);

        auto envelope = exponential_decay_envelope(steps, attenuation_factor);
        elementwise_multiply(results, envelope);

        LopassWindowedSinc lopass(results.size());
        lopass.setParams(sr * 0.49, sr);
        lopass.filter(results);

        normalize(results);
        write_sndfile("no_samp_" + output_file, {results}, sr, depth, format);

        vector<float> out_signal(output_sr * results.size() / sr);

        SRC_DATA sample_rate_info{
            results.data(), out_signal.data(), long(results.size()), long(out_signal.size()),
                0, 0, 0, output_sr / double(sr)};

        auto err = src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);

        normalize(out_signal);
        write_sndfile(output_file, {out_signal}, output_sr, depth, format);
    } catch (const cl::Error & e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const runtime_error & e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
