#pragma once

#include "glm/glm.hpp"

#include <array>

namespace cl {
class Buffer;
class CommandQueue;
}  // namespace cl

namespace wayverb {
namespace waveguide {

struct mesh_descriptor;

namespace postprocessor {

class directional_receiver final {
public:
    directional_receiver(const mesh_descriptor& mesh_descriptor,
                         double sample_rate,
                         double ambient_density,
                         size_t output_node);

    struct output final {
        glm::vec3 intensity;
        float pressure;
    };

    using return_type = output;
    return_type operator()(cl::CommandQueue& queue,
                           const cl::Buffer& buffer,
                           size_t step);

    size_t get_output_node() const;

private:
    float mesh_spacing_;
    double sample_rate_;
    double ambient_density_;
    size_t output_node_;
    std::array<unsigned, 6> surrounding_nodes_;
    glm::dvec3 velocity_{0};
};

}  // namespace postprocessor
}  // namespace waveguide
}  // namespace wayverb
