#pragma once

#include "waveguide/rectangular_waveguide.h"

namespace postprocessors {

class single_node_postprocessor
        : public rectangular_waveguide::step_postprocessor {
public:
    //  single node pressure is passed to the callback
    using output_callback = std::function<void(float)>;

    single_node_postprocessor(size_t output_node,
                              const output_callback& callback);

    void process(cl::CommandQueue& queue, const cl::Buffer& buffer) override;

private:
    size_t output_node;
    output_callback callback;
};

}  // namespace postprocessors
