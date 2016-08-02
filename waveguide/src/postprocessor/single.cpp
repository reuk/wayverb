#include "waveguide/postprocessor/single.h"

namespace waveguide {
namespace postprocessor {

single::single(size_t output_node, const output_callback& callback)
        : output_node(output_node)
        , callback(callback) {}

void single::process(cl::CommandQueue& queue, const cl::Buffer& buffer) {
    callback(read_single_value<cl_float>(queue, buffer, output_node));
}

}  // namespace postprocessor
}  // namespace waveguide
