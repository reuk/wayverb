#include "waveguide/preprocessor/gaussian.h"

#include "core/cl/common.h"
#include "core/nan_checking.h"

#include <iostream>

namespace wayverb {
namespace waveguide {
namespace preprocessor {

float gaussian::compute(const glm::vec3& x, float sdev) {
    return std::exp(-std::pow(glm::length(x), 2) / (2 * std::pow(sdev, 2))) /
           std::pow(sdev * std::sqrt(2 * M_PI), 3);
}

gaussian::gaussian(const mesh_descriptor& descriptor,
                   const glm::vec3& centre_pos,
                   float sdev,
                   size_t steps)
        : descriptor_{descriptor}
        , centre_pos_{centre_pos}
        , sdev_{sdev}
        , steps_{steps} {}

bool gaussian::operator()(cl::CommandQueue& queue,
                          cl::Buffer& buffer,
                          size_t step) const {
    if (step == steps_) {
        return false;
    }
    //  if this is the first step
    if (step == 0) {
        //  set all the mesh values

        //  first on the CPU
        const auto nodes = core::items_in_buffer<cl_float>(buffer);
        util::aligned::vector<cl_float> pressures;
        pressures.reserve(nodes);
        for (auto i = 0u; i != nodes; ++i) {
            const auto gauss = compute(
                    compute_position(descriptor_, i) - centre_pos_, sdev_);
#ifndef NDEBUG
            core::throw_if_suspicious(gauss);
#endif
            pressures.emplace_back(gauss);
        }

        //  now copy to the buffer
        cl::copy(queue, pressures.begin(), pressures.end(), buffer);
    }
    return true;
}

}  // namespace preprocessor
}  // namespace waveguide
}  // namespace wayverb
