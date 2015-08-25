#include "waveguide.h"
#include "logger.h"
#include "program.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>

using namespace std;

int main(int argc, char ** argv) {
    Logger::restart();

    vector<cl::Platform> platform;
    cl::Platform::get(&platform);

    cl_context_properties cps[3] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties) (platform[0]) (),
        0,
    };

    cl::Context context(CL_DEVICE_TYPE_GPU, cps);

    auto devices = context.getInfo<CL_CONTEXT_DEVICES>();
    auto device = devices.back();

    cl::CommandQueue queue(context, device);

    WaveguideProgram program(context, false);

    try {
        program.build({device});
        Logger::log(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));

        Waveguide waveguide(program, queue, {{100, 100, 100}});
        auto results = waveguide.run({{20, 20, 20}}, {{99, 1, 1}}, 200);

        auto mag = accumulate(results.begin(),
                              results.end(),
                              0.0f,
                              [](auto a, auto b){return max(a, fabs(b));});

        if (false && mag != 0) {
            transform(results.begin(),
                      results.end(),
                      results.begin(),
                      [mag](auto i) {return i / mag;});
        }

        cout << "{";
        for (auto & i : results) {
            cout << i << ", ";
        }
        cout << "}" << endl;

    } catch (const cl::Error & e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const runtime_error & e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
