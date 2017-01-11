#include "core/orientation.h"

#include "glm/gtx/rotate_vector.hpp"

#include <iostream>

namespace wayverb {
namespace core {

orientation::orientation(const glm::vec3& pointing, const glm::vec3& up)
        : pointing_{glm::normalize(pointing)}
        , up_{glm::normalize(up)} {}

glm::vec3 orientation::get_pointing() const { return pointing_; }

void orientation::set_pointing(const glm::vec3& u) {
    pointing_ = glm::normalize(u);
}

glm::vec3 orientation::get_up() const { return up_; }

void orientation::set_up(const glm::vec3& u) { up_ = glm::normalize(u); }

glm::mat4 orientation::get_matrix() const {
    const auto z_axis = -pointing_;
    const auto x_axis = glm::normalize(glm::cross(up_, z_axis));
    const auto y_axis = glm::normalize(glm::cross(z_axis, x_axis));
    return glm::mat4(glm::vec4(x_axis, 0),
                     glm::vec4(y_axis, 0),
                     glm::vec4(z_axis, 0),
                     glm::vec4(0, 0, 0, 1));
}

orientation combine(const orientation& a, const orientation& b) {
    const auto new_mat = a.get_matrix() * b.get_matrix();
    return orientation{glm::vec3{new_mat * glm::vec4{0, 0, -1, 0}},
                      glm::vec3{new_mat * glm::vec4{0, 1, 0, 0}}};
}

glm::vec3 transform(const orientation& orientation, const glm::vec3& vec) {
    return glm::inverse(orientation.get_matrix()) * glm::vec4{vec, 0};
}

bool operator==(const orientation& a, const orientation& b) {
    return a.get_pointing() == b.get_pointing() && a.get_up() == b.get_up();
}

bool operator!=(const orientation& a, const orientation& b) {
    return !(a == b);
}

}  // namespace wayverb
}  // namespace core
