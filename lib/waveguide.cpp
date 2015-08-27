#include "waveguide.h"

#include <iostream>

using namespace std;

Waveguide::Waveguide(const cl::Program & program,
                     cl::CommandQueue & queue,
                     cl_int3 p)
        : program(program)
        , queue(queue)
        , p(p)
        , storage({{cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * p.x * p.y * p.z),
                    cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * p.x * p.y * p.z),
                    cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * p.x * p.y * p.z)}})
        , previous(storage[0])
        , current(storage[1])
        , next(storage[2])
        , output(program.getInfo<CL_PROGRAM_CONTEXT>(),
                 CL_MEM_READ_WRITE,
                 sizeof(cl_float)) {
}

Waveguide::size_type Waveguide::get_index(cl_int3 pos) const {
    return pos.x + pos.y * p.x + pos.z * p.x * p.y;
}

vector<cl_float> Waveguide::run(cl_int3 e, cl_int3 o, int steps) {
    auto waveguide = cl::make_kernel<cl::Buffer,
                                     cl::Buffer,
                                     cl::Buffer,
                                     cl_float,
                                     cl_ulong,
                                     cl::Buffer>(program, "waveguide");

    vector<cl_float> nodes(p.x * p.y * p.z, 0);
    cl::copy(queue, nodes.begin(), nodes.end(), next);
    cl::copy(queue, nodes.begin(), nodes.end(), current);

    nodes[get_index(e)] = 1;
    cl::copy(queue, nodes.begin(), nodes.end(), previous);

    vector<cl_float> ret(steps);
    vector<cl_float> out(1);

    generate(ret.begin(),
             ret.end(),
             [this, &waveguide, &o, &out] {
                 waveguide(cl::EnqueueArgs(queue,
                                           cl::NDRange(p.x, p.y, p.z)),
                           next,
                           current,
                           previous,
                           0,
                           get_index(o),
                           output);

                 cl::copy(queue, output, out.begin(), out.end());

                 auto & temp = previous;
                 previous = current;
                 current = next;
                 next = temp;

                 return out.front();
             });

    return ret;
}
