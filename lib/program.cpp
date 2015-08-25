#include "program.h"

using namespace std;

WaveguideProgram::WaveguideProgram(const cl::Context & context, bool build_immediate)
        : Program(context, source, build_immediate) {
}

const string WaveguideProgram::source{
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

