#include "combined/model/scene.h"

#include "utilities/range.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/noise.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

namespace wayverb {
namespace combined {
namespace model {

void scene::set_visible_surface(std::experimental::optional<size_t> visible) {
    visible_surface_ = visible;
    visible_surface_changed_(get_visible_surface());
}

std::experimental::optional<size_t> scene::get_visible_surface() const {
    return visible_surface_;
}

void scene::set_origin(const glm::vec3& origin) {
    origin_ = origin;
    recompute_view_matrices();
}

glm::vec3 scene::get_origin() const { return origin_; }

void scene::set_eye_distance(float distance) {
    eye_distance_ = std::max(0.0f, distance);
    recompute_view_matrices();
}

double scene::get_eye_distance() const { return eye_distance_; }

void scene::set_rotation(const wayverb::core::az_el& az_el) {
    az_el_ = wayverb::core::az_el{
            az_el.azimuth,
            clamp(az_el.elevation,
                  util::make_range(static_cast<float>(-M_PI / 2),
                                   static_cast<float>(M_PI / 2)))};
    recompute_view_matrices();
}

wayverb::core::az_el scene::get_rotation() const { return az_el_; }

void scene::set_viewport(const glm::vec2& viewport) {
    viewport_ = viewport;
    recompute_projection_matrices();
}

glm::vec2 scene::get_viewport() const { return viewport_; }

float scene::get_aspect() const { return viewport_.x / viewport_.y; }

float scene::get_item_radius() const { return item_radius_; }

glm::mat4 scene::get_view_matrix() const { return view_matrix_; }
glm::mat4 scene::get_inverse_view_matrix() const {
    return inverse_view_matrix_;
}

glm::mat4 scene::get_projection_matrix() const { return projection_matrix_; }
glm::mat4 scene::get_inverse_projection_matrix() const {
    return inverse_projection_matrix_;
}

glm::mat4 scene::compute_projection_matrix() const {
    return glm::tweakedInfinitePerspective(45.0f, get_aspect(), 0.01f);
}

glm::mat4 scene::compute_view_matrix() const {
    const glm::vec3 from{0, 0, eye_distance_};
    const glm::vec3 target{0, 0, 0};
    const glm::vec3 up{0, 1, 0};
    return glm::lookAt(from, target, up) *
           glm::rotate(az_el_.elevation, glm::vec3(1, 0, 0)) *
           glm::rotate(az_el_.azimuth, glm::vec3(0, 1, 0)) *
           glm::translate(origin_);
}

void scene::recompute_view_matrices() {
    view_matrix_ = compute_view_matrix();
    inverse_view_matrix_ = glm::inverse(view_matrix_);
    view_matrix_changed_(get_view_matrix());
}

void scene::recompute_projection_matrices() {
    projection_matrix_ = compute_projection_matrix();
    inverse_projection_matrix_ = glm::inverse(projection_matrix_);
    projection_matrix_changed_(get_projection_matrix());
}

glm::vec3 scene::compute_world_camera_position() const {
    return inverse_view_matrix_[3];
}

glm::vec3 scene::compute_world_camera_direction() const {
    return glm::normalize(
            glm::vec3{inverse_view_matrix_ * glm::vec4{0, 0, -1, 0}});
}

scene::visible_surface_changed::connection
scene::connect_visible_surface_changed(
        visible_surface_changed::callback_type callback) {
    return visible_surface_changed_.connect(std::move(callback));
}

scene::view_matrix_changed::connection scene::connect_view_matrix_changed(
        view_matrix_changed::callback_type callback) {
    return view_matrix_changed_.connect(std::move(callback));
}

scene::projection_matrix_changed::connection
scene::connect_projection_matrix_changed(
        projection_matrix_changed::callback_type callback) {
    return projection_matrix_changed_.connect(std::move(callback));
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
