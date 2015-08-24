#include "waveguide.h"
#include "logger.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <iostream>

using namespace std;


//  TODO this should NOT be in main
const string kernel{
#ifdef DIAGNOSTIC
    "#define DIAGNOSTIC\n"
#endif
    R"(
    #define SPEED_OF_SOUND (340.0f)
    #define NULL (0)

    typedef float2 Node;

    kernel void waveguide
    (   int3 read_location
    )
    {
        size_t x = get_global_id(0);
        size_t y = get_global_id(1);
        size_t z = get_global_id(2);
    }
    )"};

int main(int argc, char ** argv) {
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

    cl::Program program(context, kernel, false);

    try {
        program.build({device});
        Logger::log(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));

        Waveguide waveguide(program, queue, 100, 100, 100);
        waveguide.iterate();
    } catch (const cl::Error & e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const runtime_error & e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
