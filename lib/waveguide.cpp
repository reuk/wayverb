#include "waveguide.h"

#include <iostream>

using namespace std;

using Node = cl_float2;

Waveguide::Waveguide(const cl::Program & program,
                     cl::CommandQueue & queue,
                     cl_int3 p)
        : program(program)
        , queue(queue)
        , p(p)
        , mesh(program.getInfo<CL_PROGRAM_CONTEXT>(),
               CL_MEM_READ_WRITE,
               sizeof(Node) * p.s[0] * p.s[1] * p.s[2]) {
}

size_t Waveguide::get_index(cl_int3 pos) const {
    return pos.s[0] + pos.s[1] * p.s[0] + pos.s[2] * p.s[0] * p.s[1];
}

vector<float> Waveguide::run(cl_int3 e, cl_int3 o, int steps) {
    auto kernel = cl::make_kernel<cl::Buffer, cl_float>(program, "waveguide");

    vector<Node> nodes(p.s[0] * p.s[1] * p.s[2], {{0, 0}});
    nodes[get_index(e)].s[1] = 1;

    cl::copy(queue, nodes.begin(), nodes.end(), mesh);

    vector<float> ret(steps);

    cout << "get_index read head: " << get_index(o) << endl;

    for (auto i = 0; i != steps; ++i) {
        kernel(cl::EnqueueArgs(queue, cl::NDRange(p.s[0], p.s[1], p.s[2])),
               mesh,
               -1);
        cl::copy(queue, mesh, nodes.begin(), nodes.end());
        ret[i] = nodes[get_index(o)].s[1];
    }

    return ret;
}
