#pragma once

#include "waveguide/waveguide.h"

namespace waveguide {
namespace postprocessor {

class single : public waveguide::step_postprocessor {
public:
    //  single node pressure is passed to the callback
    using output_callback = std::function<void(float)>;

    single(size_t output_node, const output_callback& callback);

    void process(cl::CommandQueue& queue, const cl::Buffer& buffer) override;

private:
    size_t output_node;
    output_callback callback;
};

}  // namespace postprocessor
}  // namespace waveguide
