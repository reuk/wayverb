//  project internal
#include "waveguide.h"
#include "logger.h"
#include "rectangular_program.h"
#include "tetrahedral_program.h"
#include "tetrahedral.h"
#include "scene_data.h"

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

enum class RenderType {
    TETRAHEDRAL,
    RECTANGULAR,
};

int main(int argc, char ** argv) {
    Logger::restart();

    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 3) {
        return EXIT_FAILURE;
    }

    //  load from file argv[1]
    //  save to file argv[2]

    auto speed_of_sound = 340.0;
    auto divisions = 0.2;
    auto sr = (speed_of_sound * sqrt(3)) / divisions;
    auto attenuation_factor = 0.9995;

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
        auto input = lopass_kernel(sr, sr / 4, 255);

        vector<cl_float> results;

        auto boundary = SceneData(argv[1]).get_mesh_boundary();
        auto steps = 4096;

        auto type = RenderType::TETRAHEDRAL;
        switch (type) {
            case RenderType::TETRAHEDRAL: {
                auto tetr_program =
                    get_program<TetrahedralProgram>(context, device);
                auto mesh = tetrahedral_mesh(boundary, 0, divisions);
                TetrahedralWaveguide t_waveguide(tetr_program, queue, mesh);
                results = t_waveguide.run(input, 0, 0, attenuation_factor, steps);
                break;
            }

            case RenderType::RECTANGULAR: {
                auto rect_program =
                    get_program<RectangularProgram>(context, device);
                RectangularWaveguide r_waveguide(
                    rect_program, queue, {{64, 64, 64}});
                results = r_waveguide.run(
                    input, {{20, 20, 20}}, {{35, 40, 45}}, attenuation_factor, steps);
                break;
            }
        }

        normalize(results);
        write_sndfile(fname, {results}, sr, depth, format);
    } catch (const cl::Error & e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const runtime_error & e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
