#pragma once

#include "waveguide/descriptor.h"

#include "common/cl/common.h"

namespace waveguide {
namespace preprocessor {

class gaussian final {
public:
    static float compute(const glm::vec3& x, float sdev);

    gaussian(const descriptor& descriptor,
             const glm::vec3& centre_pos,
             float sdev);

    void operator()(cl::CommandQueue& queue,
                    cl::Buffer& buffer,
                    size_t step) const;

private:
    descriptor descriptor_;
    glm::vec3 centre_pos_;
    float sdev_;
};

}  // namespace preprocessor
}  // namespace waveguide
