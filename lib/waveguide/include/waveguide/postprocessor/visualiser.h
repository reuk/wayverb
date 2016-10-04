#pragma once

#include "common/aligned/vector.h"

#include "CL/cl.hpp"

#include <functional>

namespace waveguide {
namespace postprocessor {

class visualiser final {
public:
    /// Callback is passed a complete copy of the mesh state, and the step
    using output_callback =
            std::function<void(aligned::vector<cl_float>, size_t)>;

    visualiser(output_callback callback);

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step);

private:
    output_callback callback_;
};

}  // namespace postprocessor
}  // namespace waveguide
