#include "waveguide/postprocessor/visualiser.h"

namespace waveguide {
namespace postprocessor {

visualiser::visualiser(const output_callback& callback)
        : callback(callback) {}

void visualiser::operator()(cl::CommandQueue& queue,
                            const cl::Buffer& buffer,
                            size_t step) {
    const auto nodes = buffer.getInfo<CL_MEM_SIZE>() / sizeof(cl_float);
    aligned::vector<cl_float> pressures(nodes, 0);
    cl::copy(queue, buffer, pressures.begin(), pressures.end());
    callback(pressures, step);
}

}  // namespace postprocessor
}  // namespace waveguide
