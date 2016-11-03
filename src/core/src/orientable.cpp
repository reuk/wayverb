#include "core/orientable.h"

namespace wayverb {
namespace core {

orientable::orientable(const glm::vec3& pointing)
        : pointing_{glm::normalize(pointing)} {}

glm::vec3 orientable::get_pointing() const { return pointing_; }

void orientable::set_pointing(const glm::vec3& u) {
    pointing_ = glm::normalize(u);
}

glm::mat4 orientable::get_matrix() const {
    const auto z_axis = pointing_;
    const auto x_axis =
            glm::normalize(glm::cross(pointing_, glm::vec3(0, -1, 0)));
    const auto y_axis = glm::normalize(glm::cross(z_axis, x_axis));
    return glm::mat4(glm::vec4(x_axis, 0),
                     glm::vec4(y_axis, 0),
                     glm::vec4(z_axis, 0),
                     glm::vec4(0, 0, 0, 1));
}

}  // namespace wayverb
}  // namespace core
