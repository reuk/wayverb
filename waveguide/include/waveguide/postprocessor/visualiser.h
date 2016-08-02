#pragma once

#include "waveguide/waveguide.h"

namespace waveguide {
namespace postprocessor {

class visualiser : public waveguide::step_postprocessor {
public:
    //  callback is passed a complete copy of the mesh state
    using output_callback =
            std::function<void(const aligned::vector<cl_float>&)>;

    visualiser(size_t nodes, const output_callback& callback);

    void process(cl::CommandQueue& queue, const cl::Buffer& buffer) override;

private:
    size_t nodes;
    output_callback callback;
};

}  // namespace postprocessor
}  // namespace waveguide
