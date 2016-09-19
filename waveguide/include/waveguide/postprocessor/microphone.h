#pragma once

#include "common/cl/include.h"
#include "common/output_iterator_callback.h"

#include "glm/glm.hpp"

#include <array>
#include <functional>

struct mesh_descriptor;

namespace waveguide {
namespace postprocessor {

class microphone_state final {
public:
    microphone_state(const mesh_descriptor& mesh_descriptor,
                     double sample_rate,
                     double ambient_density,
                     size_t output_node);

    struct output final {
        glm::vec3 intensity;
        float pressure;
    };

    output operator()(cl::CommandQueue& queue,
                      const cl::Buffer& buffer,
                      size_t step);

    size_t get_output_node() const;

private:
    float mesh_spacing_;
    double sample_rate_;
    double ambient_density_;
    size_t output_node_;
    std::array<cl_uint, 6> surrounding_nodes_;
    glm::dvec3 velocity_{0};
};

//----------------------------------------------------------------------------//

class microphone final {
public:
    using output_callback = std::function<void(microphone_state::output)>;

    microphone(const mesh_descriptor& mesh_descriptor,
               double sample_rate,
               double ambient_density,
               size_t output_node,
               output_callback callback);

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step);

private:
    microphone_state microphone_state_;
    output_callback callback_;
};

}  // namespace postprocessor
}  // namespace waveguide
