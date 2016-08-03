#pragma once

#include "waveguide/waveguide.h"

namespace waveguide {
namespace postprocessor {

class visualiser final {
public:
    //  callback is passed a complete copy of the mesh state
    using output_callback =
            std::function<void(const aligned::vector<cl_float>&)>;

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
