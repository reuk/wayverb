#pragma once

#include "core/cl/include.h"

namespace wayverb {
namespace waveguide {
namespace postprocessor {

class node final {
public:
    node(size_t output_node);

    using return_type = float;
    return_type operator()(cl::CommandQueue& queue,
                           const cl::Buffer& buffer,
                           size_t step) const;

    size_t get_output_node() const;

private:
    size_t output_node_;
};

}  // namespace postprocessor
}  // namespace waveguide
}  // namespace wayverb
