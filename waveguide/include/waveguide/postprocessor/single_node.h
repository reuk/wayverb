#pragma once

#include "waveguide/waveguide.h"

namespace waveguide {
namespace postprocessor {

class node_state final {
public:
    node_state(size_t output_node);

    using value_type = float;
    value_type operator()(cl::CommandQueue& queue,
                          const cl::Buffer& buffer,
                          size_t step) const;

    size_t get_output_node() const;

private:
    size_t output_node;
};

}  // namespace postprocessor
}  // namespace waveguide