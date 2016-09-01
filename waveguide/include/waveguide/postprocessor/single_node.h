#pragma once

#include "waveguide/waveguide.h"

namespace waveguide {
namespace postprocessor {

namespace detail {
class node_state final {
public:
    node_state(size_t output_node);

    float operator()(cl::CommandQueue& queue,
                     const cl::Buffer& buffer,
                     size_t step);

    size_t get_output_node() const;

private:
    size_t output_node;
};
}  // namespace detail

class node final {
public:
    using output_callback = std::function<void(float)>;

    node(size_t output_node, output_callback callback);

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step);

private:
    detail::node_state node_state_;
    output_callback callback_;
};

}  // namespace postprocessor
}  // namespace waveguide
