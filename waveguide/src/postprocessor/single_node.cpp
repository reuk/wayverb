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

node::node(size_t output_node, const output_callback& callback)
        : node_state(output_node)
        , callback(callback) {}

void node::operator()(cl::CommandQueue& queue,
                             const cl::Buffer& buffer,
                             size_t step) {
    callback(node_state(queue, buffer, step));
}

multi_node::multi_node(aligned::vector<size_t> output_node,
                       const output_callback& callback)
        : state(map_to_vector(
                  output_node,
                  [](auto i) { return detail::node_state(i); }))
        , callback(callback) {}

void multi_node::operator()(cl::CommandQueue& queue,
                            const cl::Buffer& buffer,
                            size_t step) {
    aligned::vector<std::tuple<float, size_t>> ret;
    ret.reserve(state.size());
    for (auto& i : state) {
        ret.push_back(
                std::make_tuple(i(queue, buffer, step), i.get_output_node()));
    }
    callback(ret);
}

}  // namespace postprocessor
}  // namespace waveguide
