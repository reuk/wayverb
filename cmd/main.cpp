//  project internal
#include "waveguide.h"
#include "scene_data.h"
#include "logger.h"
#include "test_flag.h"
#include "filters.h"
#include "sample_rate_conversion.h"

//  dependency
#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "sndfile.hh"

#include <gflags/gflags.h>

//  stdlib
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>

using namespace std;

vector<float> exponential_decay_envelope(int steps, float attenuation_factor) {
    vector<float> ret(steps);
    auto amp = 1.0f;
    generate(begin(ret),
             end(ret),
             [&amp, attenuation_factor] {
                 auto t = amp;
                 amp *= attenuation_factor;
                 return t;});
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

void write_sndfile(const string & fname,
                   const vector<vector<float>> & outdata,
                   float sr,
                   unsigned long bd,
                   unsigned long ftype) {
    vector<float> interleaved(outdata.size() * outdata[0].size());

    for (auto i = 0u; i != outdata.size(); ++i)
        for (auto j = 0u; j != outdata[i].size(); ++j)
            interleaved[j * outdata.size() + i] = outdata[i][j];

    SndfileHandle outfile(fname, SFM_WRITE, ftype | bd, outdata.size(), sr);
    outfile.write(interleaved.data(), interleaved.size());
}

void print_device_info(const cl::Device & i) {
    Logger::log(i.getInfo<CL_DEVICE_NAME>());
    Logger::log("available: ", i.getInfo<CL_DEVICE_AVAILABLE>());
};

cl::Context get_context() {
    vector<cl::Platform> platform;
    cl::Platform::get(&platform);

    cl_context_properties cps[3] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)(platform[0])(), 0,
    };

    return cl::Context(CL_DEVICE_TYPE_GPU, cps);
}

cl::Device get_device(const cl::Context & context) {
    auto devices = context.getInfo<CL_CONTEXT_DEVICES>();

    Logger::log("## all devices:");

    for (auto & i : devices) {
        print_device_info(i);
    }

    auto device = devices.back();

    Logger::log("## used device:");
    print_device_info(device);

    return device;
}

template <typename T>
T get_program(const cl::Context & context, const cl::Device & device) {
    T program(context);
    try {
        program.build({device});
    } catch (const cl::Error & e) {
        Logger::log(
            program.template getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
        throw;
    }
    return program;
}

auto get_file_format(const string & fname) {
    map<string, unsigned long> ftypeTable{{"aif", SF_FORMAT_AIFF},
                                          {"aiff", SF_FORMAT_AIFF},
                                          {"wav", SF_FORMAT_WAV}};

    auto extension = fname.substr(fname.find_last_of(".") + 1);
    auto ftypeIt = ftypeTable.find(extension);
    if (ftypeIt == ftypeTable.end()) {
        stringstream ss;
        ss << "Invalid output file extension - valid extensions are: ";
        for (const auto & i : ftypeTable)
            ss << i.first << " ";
        throw runtime_error(ss.str());
    }
    return ftypeIt->second;
}

auto get_file_depth(unsigned long bitDepth) {
    map<unsigned long, unsigned long> depthTable{{16, SF_FORMAT_PCM_16},
                                                 {24, SF_FORMAT_PCM_24}};

    auto depthIt = depthTable.find(bitDepth);
    if (depthIt == depthTable.end()) {
        stringstream ss;
        ss << "Invalid bitdepth - valid bitdepths are: ";
        for (const auto & i : depthTable)
            ss << i.first << " ";
        throw runtime_error(ss.str());
    }
    return depthIt->second;
}

int main(int argc, char ** argv) {
    Logger::restart();
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 3) {
        Logger::log_err("expecting an input and an output filename");
        return EXIT_FAILURE;
    }

    //  load from file argv[1]
    //  save to file argv[2]

    auto speed_of_sound = 340.0;
    auto attenuation_factor = 0.999;

    auto max_freq = 500;
    auto sr = max_freq * 4;
    auto divisions = (speed_of_sound * sqrt(3)) / sr;

    auto output_sr = 44100;

    auto steps = 1 << 13;

    string fname(argv[2]);
    auto bitDepth = 16;

    unsigned long format, depth;

    try {
        format = get_file_format(fname);
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

        auto boundary = SceneData(argv[1]).get_mesh_boundary();

        auto program = get_program<TetrahedralProgram>(context, device);
        IterativeTetrahedralWaveguide waveguide(
            program, queue, boundary, divisions);
        results = waveguide.run(20000, 20000, steps);

        auto envelope = exponential_decay_envelope(steps, attenuation_factor);
        elementwise_multiply(results, envelope);

        //  TODO ew ugly
        auto kernel = lopass_kernel(sr, 0.24, 127);
        FastConvolution convolver(kernel.size() + results.size() - 1);
        auto filtered = convolver.convolve(results, kernel);

        normalize(filtered);
        auto out_signal = convert_sample_rate(filtered, output_sr, sr);
        write_sndfile(fname, {out_signal}, output_sr, depth, format);
    } catch (const cl::Error & e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const runtime_error & e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
