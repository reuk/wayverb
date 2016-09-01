#include "waveguide/preprocessor/gaussian.h"

#include "common/nan_checking.h"

namespace waveguide {
namespace preprocessor {

float gaussian::compute(const glm::vec3& x, float sdev) {
    return 1 / std::pow(sdev * std::sqrt(2 * M_PI), 3) *
           std::exp(-std::pow(glm::length(x), 2) / (2 * std::pow(sdev, 2)));
}

gaussian::gaussian(const descriptor& descriptor,
                   const glm::vec3& centre_pos,
                   float sdev)
        : descriptor_(descriptor)
        , centre_pos_(centre_pos)
        , sdev_(sdev) {}

void gaussian::operator()(cl::CommandQueue& queue,
                          cl::Buffer& buffer,
                          size_t step) const {
    //  if this is the first step
    if (step == 0) {
        //  set all the mesh values

        //  first on the CPU
        const auto nodes{items_in_buffer<cl_float>(buffer)};
        aligned::vector<cl_float> pressures;
        pressures.reserve(nodes);
        for (auto i{0u}; i != nodes; ++i) {
            const auto gauss{compute(
                    centre_pos_ - compute_position(descriptor_, i), sdev_)};
            throw_if_suspicious(gauss);
            pressures.push_back(gauss);
        }

        //  now copy to the buffer
        cl::copy(queue, pressures.begin(), pressures.end(), buffer);
    }
}

}  // namespace preprocessor
}  // namespace waveguide
