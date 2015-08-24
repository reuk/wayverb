#include "waveguide.h"

using namespace std;

Waveguide::Waveguide(const cl::Program & program, cl::CommandQueue & queue, int x, int y, int z)
        : program(program)
        , queue(queue)
        , x(x)
        , y(y)
        , z(z)
        , mesh(/*TODO*/) {

}

void Waveguide::iterate() const {
    cl_int3 in {{10, 10, 10}};

    auto kernel = cl::make_kernel<cl_int3>(program, "waveguide");

    kernel(cl::EnqueueArgs(queue, cl::NDRange(x, y, z)), in);
}
