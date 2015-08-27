#include "waveguide.h"
#include "logger.h"
#include "program.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "sndfile.hh"

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>

using namespace std;

void write_sndfile(const string & fname,
                   const vector<vector<float>> & outdata,
                   float sr,
                   unsigned long bd,
                   unsigned long ftype) {
    vector<float> interleaved(outdata.size() * outdata[0].size());

    for (auto i = 0; i != outdata.size(); ++i)
        for (auto j = 0; j != outdata[i].size(); ++j)
            interleaved[j * outdata.size() + i] = outdata[i][j];

    SndfileHandle outfile(fname, SFM_WRITE, ftype | bd, outdata.size(), sr);
    outfile.write(interleaved.data(), interleaved.size());
}

void print_device_info(const cl::Device & i) {
    cout << i.getInfo<CL_DEVICE_NAME>() << endl;
    cout << "available: " << i.getInfo<CL_DEVICE_AVAILABLE>() << endl;
    cout << "compute units: " << i.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()
         << endl;
    auto dim = i.getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>();
    cout << "work dimensions: " << dim << endl;
    cout << "work items: ";
    for (auto j = 0; j != dim; ++j)
        cout << i.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[j] << ", ";
    cout << endl;
    cout << endl;
};

int main(int argc, char ** argv) {
    Logger::restart();

    auto speed_of_sound = 340.0;
    auto divisions = 0.2;
    auto sr = (speed_of_sound * sqrt(3)) / divisions;

    string fname{"sndfile.aiff"};
    auto bitDepth = 16;

    map<unsigned long, unsigned long> depthTable{{16, SF_FORMAT_PCM_16},
                                                 {24, SF_FORMAT_PCM_24}};

    auto depthIt = depthTable.find(bitDepth);
    if (depthIt == depthTable.end()) {
        cerr << "Invalid bitdepth - valid bitdepths are: ";
        for (const auto & i : depthTable)
            cerr << i.first << " ";
        cerr << endl;
        return EXIT_FAILURE;
    }

    map<string, unsigned long> ftypeTable{{"aif", SF_FORMAT_AIFF},
                                          {"aiff", SF_FORMAT_AIFF},
                                          {"wav", SF_FORMAT_WAV}};

    auto extension = fname.substr(fname.find_last_of(".") + 1);
    auto ftypeIt = ftypeTable.find(extension);
    if (ftypeIt == ftypeTable.end()) {
        cerr << "Invalid output file extension - valid extensions are: ";
        for (const auto & i : ftypeTable)
            cerr << i.first << " ";
        cerr << endl;
        return EXIT_FAILURE;
    }

    vector<cl::Platform> platform;
    cl::Platform::get(&platform);

    cl_context_properties cps[3] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)(platform[0])(), 0,
    };

    cl::Context context(CL_DEVICE_TYPE_GPU, cps);

    auto devices = context.getInfo<CL_CONTEXT_DEVICES>();

    cout << "## all devices:" << endl;

    for (auto & i : devices) {
        print_device_info(i);
    }

    auto device = devices.back();

    cout << "## used device:" << endl;
    print_device_info(device);

    cl::CommandQueue queue(context, device);

    WaveguideProgram program(context, false);

    try {
        program.build({device});
        Logger::log(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));

        Waveguide waveguide(program, queue, {{64, 64, 64}});
        auto results = waveguide.run({{20, 20, 20}}, {{30, 30, 30}}, 4096);

        auto mag = accumulate(results.begin(),
                              results.end(),
                              0.0f,
                              [](auto a, auto b) { return max(a, fabs(b)); });

        if (mag != 0) {
            transform(results.begin(),
                      results.end(),
                      results.begin(),
                      [mag](auto i) { return i / mag; });
        }

        write_sndfile(fname, {results}, sr, depthIt->second, ftypeIt->second);

    } catch (const cl::Error & e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const runtime_error & e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
