#pragma once

#include "waveguide/rectangular_waveguide.h"

namespace postprocessors {

class visualiser_step_postprocessor
        : public rectangular_waveguide::step_postprocessor {
public:
    //  callback is passed a complete copy of the mesh state
    using output_callback =
            std::function<void(const aligned::vector<cl_float>&)>;

    visualiser_step_postprocessor(size_t nodes,
                                  const output_callback& callback);

    void process(cl::CommandQueue& queue, const cl::Buffer& buffer) override;

private:
    size_t nodes;
    output_callback callback;
};

}  // namespace postprocessors
