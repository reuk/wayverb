#include "waveguide/postprocessor/visualiser.h"

namespace waveguide {
namespace postprocessor {

visualiser::visualiser(size_t nodes, const output_callback& callback)
        : nodes(nodes) {}

void visualiser::process(cl::CommandQueue& queue, const cl::Buffer& buffer) {
    aligned::vector<cl_float> pressures(nodes, 0);
    cl::copy(queue, buffer, pressures.begin(), pressures.end());
    callback(pressures);
}

}  // namespace postprocessor
}  // namespace waveguide
