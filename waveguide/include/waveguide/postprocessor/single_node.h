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

template <typename It>
class node final {
public:
    node(size_t output_node, It output_iterator)
            : node_state_(output_node)
            , output_iterator_(std::move(output_iterator)) {}

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step) {
        *output_iterator_++ = node_state_(queue, buffer, step);
    }

private:
    detail::node_state node_state_;
    It output_iterator_;
};

template <typename It>
auto make_node(size_t output_node, It output_iterator) {
    return node<It>{output_node, output_iterator};
}

}  // namespace postprocessor
}  // namespace waveguide
