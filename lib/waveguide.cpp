#include "waveguide.h"

#include <iostream>
#include <algorithm>

using namespace std;

RectangularWaveguide::RectangularWaveguide(const RectangularProgram & program,
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

RectangularWaveguide::size_type RectangularWaveguide::get_index(cl_int3 pos) const {
    return pos.x + pos.y * p.x + pos.z * p.x * p.y;
}

vector<cl_float> RectangularWaveguide::run(const vector<float> & input,
                                cl_int3 e,
                                cl_int3 o,
                                int steps) {
    auto waveguide = cl::make_kernel<cl_ulong,
                                     cl_float,
                                     cl::Buffer,
                                     cl::Buffer,
                                     cl::Buffer,
                                     cl_float,
                                     cl_ulong,
                                     cl::Buffer>(program, "waveguide");

    vector<cl_float> nodes(p.x * p.y * p.z, 0);
    cl::copy(queue, nodes.begin(), nodes.end(), next);
    cl::copy(queue, nodes.begin(), nodes.end(), current);
    cl::copy(queue, nodes.begin(), nodes.end(), previous);

    auto in_copy = input;
    in_copy.resize(steps, 0);

    vector<cl_float> ret(in_copy.size());
    vector<cl_float> out(1);

    transform(in_copy.begin(),
              in_copy.end(),
              ret.begin(),
              [this, &waveguide, &e, &o, &out](auto i) {
                  waveguide(cl::EnqueueArgs(queue, cl::NDRange(p.x, p.y, p.z)),
                            this->get_index(e),
                            i,
                            next,
                            current,
                            previous,
                            -1,
                            this->get_index(o),
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

TetrahedralWaveguide::TetrahedralWaveguide(const TetrahedralProgram & program,
                     cl::CommandQueue & queue,
                     const vector<Node> & nodes)
        : program(program)
        , queue(queue)
        , nodes(nodes)
        , storage({{cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * nodes.size()),
                    cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * nodes.size()),
                    cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * nodes.size())}})
        , previous(storage[0])
        , current(storage[1])
        , next(storage[2])
        , output(program.getInfo<CL_PROGRAM_CONTEXT>(),
                 CL_MEM_READ_WRITE,
                 sizeof(cl_float)) {
}

vector<cl_float> TetrahedralWaveguide::run(const std::vector<float> & input,
                                           size_type excitation,
                                           size_type read_head,
                                           int steps) {
    vector<cl_float> ret;
    return ret;
}
