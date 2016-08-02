#include "waveguide/postprocessors/visualiser.h"

namespace postprocessors {

visualiser_step_postprocessor::visualiser_step_postprocessor(
        size_t nodes, const output_callback& callback)
        : nodes(nodes) {}

void visualiser_step_postprocessor::process(cl::CommandQueue& queue,
                                            const cl::Buffer& buffer) {
    aligned::vector<cl_float> pressures(nodes, 0);
    cl::copy(queue, buffer, pressures.begin(), pressures.end());
    callback(pressures);
}

}  // namespace postprocessors
