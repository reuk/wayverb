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

node::node(size_t output_node, output_callback callback)
        : node_state_(output_node)
        , callback_(std::move(callback)) {}

void node::operator()(cl::CommandQueue& queue,
                      const cl::Buffer& buffer,
                      size_t step) {
    callback_(node_state_(queue, buffer, step));
}

}  // namespace postprocessor
}  // namespace waveguide
