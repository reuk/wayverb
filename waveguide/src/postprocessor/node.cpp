#include "waveguide/postprocessor/node.h"

#include "common/cl/common.h"
#include "common/map_to_vector.h"

namespace waveguide {
namespace postprocessor {

node::node(size_t output_node)
        : output_node_{output_node} {}

float node::operator()(cl::CommandQueue& queue,
                       const cl::Buffer& buffer,
                       size_t step) const {
    return read_value<cl_float>(queue, buffer, output_node_);
}

size_t node::get_output_node() const { return output_node_; }

}  // namespace postprocessor
}  // namespace waveguide
