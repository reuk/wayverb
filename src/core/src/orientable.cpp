#include "core/orientable.h"

#include "glm/gtx/rotate_vector.hpp"

namespace wayverb {
namespace core {

orientable::orientable(const glm::vec3& pointing, const glm::vec3& up)
        : pointing_{glm::normalize(pointing)}
        , up_{glm::normalize(up)} {}

glm::vec3 orientable::get_pointing() const { return pointing_; }

void orientable::set_pointing(const glm::vec3& u) {
    pointing_ = glm::normalize(u);
}

glm::vec3 orientable::get_up() const { return up_; }

void orientable::set_up(const glm::vec3& u) { up_ = glm::normalize(u); }

glm::mat4 orientable::get_matrix() const {
    const auto z_axis = pointing_;
    const auto x_axis = glm::normalize(glm::cross(up_, pointing_));
    const auto y_axis = glm::normalize(glm::cross(z_axis, x_axis));
    return glm::mat4(glm::vec4(x_axis, 0),
                     glm::vec4(y_axis, 0),
                     glm::vec4(z_axis, 0),
                     glm::vec4(0, 0, 0, 1));
}

orientable combine(const orientable& a, const orientable& b) {
    const auto new_mat = a.get_matrix() * b.get_matrix();
    return orientable{glm::vec3{new_mat * glm::vec4{0, 0, 1, 0}},
                      glm::vec3{new_mat * glm::vec4{0, 1, 0, 0}}};
}

glm::vec3 transform(const orientable& orientable, const glm::vec3& vec) {
    return glm::inverse(orientable.get_matrix()) * glm::vec4{vec, 0};
}

}  // namespace wayverb
}  // namespace core
