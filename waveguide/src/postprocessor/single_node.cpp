#include "waveguide/postprocessor/single_node.h"

#include "common/cl/common.h"
#include "common/map_to_vector.h"

namespace waveguide {
namespace postprocessor {
node_state::node_state(size_t output_node)
        : output_node{output_node} {}

float node_state::operator()(cl::CommandQueue& queue,
                             const cl::Buffer& buffer,
                             size_t step) const {
    return read_single_value<cl_float>(queue, buffer, output_node);
}

size_t node_state::get_output_node() const { return output_node; }
}  // namespace postprocessor
}  // namespace waveguide