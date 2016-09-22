#pragma once

#include "common/cl/include.h"

namespace waveguide {
namespace postprocessor {

class node final {
public:
    node(size_t output_node);

    using value_type = float;
    value_type operator()(cl::CommandQueue& queue,
                          const cl::Buffer& buffer,
                          size_t step) const;

    size_t get_output_node() const;

private:
    size_t output_node_;
};

}  // namespace postprocessor
}  // namespace waveguide
