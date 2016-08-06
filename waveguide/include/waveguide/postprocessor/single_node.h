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
    //  node pressure is passed to the callback
    using output_callback = std::function<void(float)>;

    node(size_t output_node, const output_callback& callback);

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step);

private:
    detail::node_state node_state;
    output_callback callback;
};

class multi_node final {
public:
    //  node pressure is passed to the callback
    using output_callback = std::function<void(
            const aligned::vector<std::tuple<float, size_t>>&)>;

    multi_node(aligned::vector<size_t> output_node,
               const output_callback& callback);

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step);

private:
    aligned::vector<detail::node_state> state;
    output_callback callback;
};

}  // namespace postprocessor
}  // namespace waveguide

