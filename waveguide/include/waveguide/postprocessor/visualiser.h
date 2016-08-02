#pragma once

#include "waveguide/waveguide.h"

namespace waveguide {
namespace postprocessor {

class visualiser final {
public:
    //  callback is passed a complete copy of the mesh state
    using output_callback =
            std::function<void(const aligned::vector<cl_float>&)>;

    visualiser(const output_callback& callback);

    void operator()(cl::CommandQueue& queue, const cl::Buffer& buffer);

private:
    output_callback callback;
};

}  // namespace postprocessor
}  // namespace waveguide
