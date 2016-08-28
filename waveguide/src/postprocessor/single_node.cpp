#include "waveguide/postprocessor/single_node.h"

#include "common/map_to_vector.h"

namespace waveguide {
namespace postprocessor {
namespace detail {
node_state::node_state(size_t output_node)
        : output_node(output_node) {}

float node_state::operator()(cl::CommandQueue& queue,
                             const cl::Buffer& buffer,
                             size_t step) {
    return read_single_value<cl_float>(queue, buffer, output_node);
}

size_t node_state::get_output_node() const { return output_node; }
}  // namespace detail
}  // namespace postprocessor
}  // namespace waveguide
