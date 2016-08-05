#pragma once

#include "waveguide/waveguide.h"

namespace waveguide {
namespace postprocessor {

class visualiser final {
public:
    /// Callback is passed a complete copy of the mesh state, and the step
    using output_callback =
            std::function<void(const aligned::vector<cl_float>&, size_t)>;

    template <typename T>
    visualiser(T&& t)
            : callback(std::forward<T>(t)) {}

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step);

private:
    output_callback callback;
};

}  // namespace postprocessor
}  // namespace waveguide
