//  project internal
#include "waveguide.h"
#include "scene_data.h"
#include "test_flag.h"
#include "conversions.h"

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

// -1 <= z <= 1, -pi <= theta <= pi
cl_float3 spherePoint(float z, float theta) {
    const float ztemp = sqrtf(1 - z * z);
    return (cl_float3){{ztemp * cosf(theta), ztemp * sinf(theta), z, 0}};
}

vector<cl_float3> getRandomDirections(unsigned long num) {
    vector<cl_float3> ret(num);
    uniform_real_distribution<float> zDist(-1, 1);
    uniform_real_distribution<float> thetaDist(-M_PI, M_PI);
    auto seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine engine(seed);

    for (auto && i : ret)
        i = spherePoint(zDist(engine), thetaDist(engine));

    return ret;
}

double a2db(double a) {
    return 20 * log10(a);
}

double db2a(double db) {
    return pow(10, db / 20);
}

vector<float> squintegrate(const std::vector<float> & sig) {
    vector<float> ret(sig.size());
    partial_sum(sig.rbegin(),
                sig.rend(),
                ret.rbegin(),
                [](auto i, auto j) { return i + j * j; });
    return ret;
}

int rt60(const vector<float> & sig) {
    auto squintegrated = squintegrate(sig);
    normalize(squintegrated);
    auto target = db2a(-60);
    return distance(squintegrated.begin(),
                    find_if(squintegrated.begin(),
                            squintegrated.end(),
                            [target](auto i) { return i < target; }));
}

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

    auto output_sr = 44100;
    auto bitDepth = 16;

    unsigned long format, depth;

    try {
        format = get_file_format(output_file);
        depth = get_file_depth(bitDepth);
    } catch (const runtime_error & e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    }

    //  global params
    auto speed_of_sound = 340.0;

    auto max_freq = 500;
    auto filter_freq = max_freq * 0.9;
    auto sr = max_freq * 4;
    auto divisions = (speed_of_sound * sqrt(3)) / sr;

    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);

    auto num_rays = 1024 * 8;
    auto num_impulses = 64;

    vector<vector<float>> raytrace_results;
    auto directions = getRandomDirections(num_rays);
    cl_float3 source{{0, 2, 0}};
    cl_float3 mic{{0, 2, 1}};
    try {
        auto program = get_program<RayverbProgram>(context, device);
        Raytrace raytrace(
            program, queue, num_impulses, model_file, material_file);
        raytrace.raytrace(mic, source, directions);
        auto results = raytrace.getAllRaw(false);
        vector<Speaker> speakers{{Speaker{cl_float3{{0, 0, 0}}, 0}}};
        auto attenuated =
            Attenuate(program, queue).attenuate(results, speakers);
        //        fixPredelay(attenuated);

        auto flattened = flattenImpulses(attenuated, output_sr);
        raytrace_results = process(FILTER_TYPE_BIQUAD_ONEPASS,
                                   flattened,
                                   output_sr,
                                   true,
                                   true,
                                   true,
                                   1.0);

        LinkwitzRiley hipass;
        hipass.setParams(filter_freq, output_sr * 0.45, output_sr);
        for (auto & i : raytrace_results)
            hipass.filter(i);
        normalize(raytrace_results);

        write_sndfile("raytrace_" + output_file,
                      raytrace_results,
                      output_sr,
                      depth,
                      format);
    } catch (const cl::Error & e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const runtime_error & e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        Logger::log_err("unknown error");
        return EXIT_FAILURE;
    }

    auto decay_frames = rt60(raytrace_results.front());
    auto attenuation_factor = pow(db2a(-60), 1.0 / decay_frames);
    Logger::log("attenuation factor: ", attenuation_factor);

#ifdef TESTING
    auto steps = 1 << 8;
#else
    auto steps = 1 << 13;
#endif

    vector<vector<cl_float>> waveguide_results;
    try {
        auto boundary = get_mesh_boundary(SceneData(model_file, material_file));

        auto program = get_program<TetrahedralProgram>(context, device);
        IterativeTetrahedralWaveguide waveguide(
            program, queue, boundary, divisions);
        auto source_index = waveguide.get_index_for_coordinate(convert(source));
        auto mic_index = waveguide.get_index_for_coordinate(convert(mic));
        Logger::log("index: ", index);
        auto results = waveguide.run(source_index, mic_index, steps);

        auto envelope = exponential_decay_envelope(steps, attenuation_factor);
        elementwise_multiply(results, envelope);
        normalize(results);

        vector<float> out_signal(output_sr * results.size() / sr);

        SRC_DATA sample_rate_info{results.data(),
                                  out_signal.data(),
                                  long(results.size()),
                                  long(out_signal.size()),
                                  0,
                                  0,
                                  0,
                                  output_sr / double(sr)};

        src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);

        LinkwitzRiley lopass;
        lopass.setParams(1, filter_freq, output_sr);
        lopass.filter(out_signal);

        normalize(out_signal);

        waveguide_results = {out_signal};

        write_sndfile("waveguide_" + output_file,
                      waveguide_results,
                      output_sr,
                      depth,
                      format);
    } catch (const cl::Error & e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const runtime_error & e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        Logger::log_err("unknown error");
        return EXIT_FAILURE;
    }

    auto max_index =
        max(raytrace_results.front().size(), waveguide_results.front().size());
    vector<float> summed_results(max_index, 0);
    for (auto j = 0u; j != raytrace_results.size(); ++j) {
        for (auto i = 0u; i != max_index; ++i) {
            if (i < raytrace_results[j].size())
                summed_results[i] += raytrace_results[j][i];
            if (i < waveguide_results.size())
                summed_results[i] += waveguide_results[j][i];
        }
    }
    write_sndfile(
        "summed_" + output_file, {summed_results}, output_sr, depth, format);

    return EXIT_SUCCESS;
}
