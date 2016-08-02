#include "waveguide/postprocessors/single.h"

namespace postprocessors {

single_node_postprocessor::single_node_postprocessor(
        size_t output_node, const output_callback& callback)
        : output_node(output_node)
        , callback(callback) {}

void single_node_postprocessor::process(cl::CommandQueue& queue,
                                        const cl::Buffer& buffer) {
    callback(read_single_value<cl_float>(queue, buffer, output_node));
}

}  // namespace postprocessors
