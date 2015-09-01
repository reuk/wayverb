#include "waveguide.h"
#include "logger.h"
#include "program.h"
#include "tetrahedral.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "sndfile.hh"

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>

using namespace std;

template <typename T>
auto max_mag(const T & t) {
    return accumulate(t.begin(),
                      t.end(),
                      typename T::value_type(0),
                      [](auto a, auto b) { return max(a, fabs(b)); });
}

template <typename T>
void normalize(T & t) {
    auto mag = max_mag(t);
    if (mag != 0) {
        transform(
            t.begin(), t.end(), t.begin(), [mag](auto i) { return i / mag; });
    }
}

/// sinc t = sin (pi . t) / pi . t
template <typename T>
T sinc(const T & t) {
    T pit = M_PI * t;
    return sin(pit) / pit;
}

/// Generate a convolution kernel for a lowpass sinc filter (NO WINDOWING!).
template <typename T = float>
vector<T> sinc_kernel(double cutoff, unsigned long length) {
    if (!(length % 2))
        throw runtime_error("Length of sinc filter kernel must be odd.");

    vector<T> ret(length);
    for (auto i = 0u; i != length; ++i) {
        if (i == ((length - 1) / 2))
            ret[i] = 1;
        else
            ret[i] = sinc(2 * cutoff * (i - (length - 1) / 2.0));
    }
    return ret;
}

/// Generate a blackman window of a specific length.
template <typename T = float>
vector<T> blackman(unsigned long length) {
    const auto a0 = 7938.0 / 18608.0;
    const auto a1 = 9240.0 / 18608.0;
    const auto a2 = 1430.0 / 18608.0;

    vector<T> ret(length);
    for (auto i = 0u; i != length; ++i) {
        const auto offset = i / (length - 1.0);
        ret[i] =
            (a0 - a1 * cos(2 * M_PI * offset) + a2 * cos(4 * M_PI * offset));
    }
    return ret;
}

/// Generate a windowed, normalized low-pass sinc filter kernel of a specific
/// length.
template <typename T = float>
vector<T> lopass_kernel(float sr, float cutoff, unsigned long length) {
    auto window = blackman<T>(length);
    auto kernel = sinc_kernel<T>(cutoff / sr, length);
    transform(begin(window),
              end(window),
              begin(kernel),
              begin(kernel),
              [](auto i, auto j) { return i * j; });
    normalize(kernel);
    return kernel;
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

WaveguideProgram get_program(const cl::Context & context,
                             const cl::Device & device) {
    WaveguideProgram program(context);
    try {
        program.build({device});
    } catch (const cl::Error & e) {
        Logger::log(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
        throw;
    }
    return program;
}

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

    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);
    WaveguideProgram program(context, false);

    try {
        auto program = get_program(context, device);

        auto input = lopass_kernel(sr, sr / 4, 255);

        Waveguide waveguide(program, queue, {{128, 64, 64}});
        auto results =
            waveguide.run(input, {{20, 20, 20}}, {{35, 40, 45}}, 4096);

        normalize(results);

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
