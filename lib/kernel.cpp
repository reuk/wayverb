#include "kernel.h"

#include "logger.h"

using namespace std;

Kernel::Kernel(const cl::Context & context,
               const cl::Device & device,
               const string & kernel,
               bool verbose)
        : program(context, kernel, false) {
    program.build({device});

    if (verbose) {
        Logger::log(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
    }
}

WaveguideKernel::WaveguideKernel(const cl::Context & context,
                                 const cl::Device & device,
                                 bool verbose)
        : Kernel(context, device, kernel, verbose) {
}

const string WaveguideKernel::kernel{
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
