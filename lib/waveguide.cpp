#include "waveguide.h"

#include <iostream>

using namespace std;

Waveguide::Waveguide(const cl::Program & program,
                     cl::CommandQueue & queue,
                     cl_int3 p)
        : program(program)
        , queue(queue)
        , p(p)
        , next(program.getInfo<CL_PROGRAM_CONTEXT>(),
               CL_MEM_WRITE_ONLY,
               sizeof(cl_float) * p.s[0] * p.s[1] * p.s[2])
        , current(program.getInfo<CL_PROGRAM_CONTEXT>(),
                  CL_MEM_READ_ONLY,
                  sizeof(cl_float) * p.s[0] * p.s[1] * p.s[2])
        , previous(program.getInfo<CL_PROGRAM_CONTEXT>(),
                   CL_MEM_READ_ONLY,
                   sizeof(cl_float) * p.s[0] * p.s[1] * p.s[2]) {
}

size_t Waveguide::get_index(cl_int3 pos) const {
    return pos.s[0] + pos.s[1] * p.s[0] + pos.s[2] * p.s[0] * p.s[1];
}

vector<float> Waveguide::run(cl_int3 e, cl_int3 o, int steps) {
    auto waveguide =
        cl::make_kernel<cl::Buffer, cl::Buffer, cl::Buffer, cl_float>(
            program, "waveguide");

    vector<cl_float> nodes(p.s[0] * p.s[1] * p.s[2], 0);
    cl::copy(queue, nodes.begin(), nodes.end(), previous);
    cl::copy(queue, nodes.begin(), nodes.end(), next);

    nodes[get_index(e)] = 1;
    cl::copy(queue, nodes.begin(), nodes.end(), current);

    vector<float> ret(steps);

    for (auto i = 0; i != steps; ++i) {
        cl::NDRange range(p.s[0], p.s[1], p.s[2]);
        waveguide(cl::EnqueueArgs(queue, range), next, current, previous, 1);

        cl::copy(queue, next, nodes.begin(), nodes.end());

        ret[i] = nodes[get_index(o)];

        previous = current;
        current = next;
    }

    return ret;
}
