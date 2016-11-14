#include "combined/model/scene.h"

#include "utilities/range.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/noise.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

namespace wayverb {
namespace combined {
namespace model {

view_state::view_state(const glm::vec3& origin,
                       float eye_distance,
                       const wayverb::core::az_el& az_el)
        : origin_{origin}
        , eye_distance_{eye_distance}
        , az_el_{az_el} {}

void view_state::set_origin(const glm::vec3& origin) { origin_ = origin; }
glm::vec3 view_state::get_origin() const { return origin_; }

void view_state::set_eye_distance(float distance) {
    eye_distance_ = std::max(0.1f, distance);
}
float view_state::get_eye_distance() const { return eye_distance_; }

void view_state::set_rotation(const wayverb::core::az_el& az_el) {
    az_el_ = wayverb::core::az_el{
            az_el.azimuth,
            clamp(az_el.elevation,
                  util::make_range(static_cast<float>(-M_PI / 2),
                                   static_cast<float>(M_PI / 2)))};
}
wayverb::core::az_el view_state::get_rotation() const { return az_el_; }

glm::mat4 get_view_matrix(const view_state& view_state) {
    const glm::vec3 from{0, 0, view_state.get_eye_distance()};
    const glm::vec3 target{0, 0, 0};
    const glm::vec3 up{0, 1, 0};
    return glm::lookAt(from, target, up) *
           glm::rotate(view_state.get_rotation().elevation,
                       glm::vec3(1, 0, 0)) *
           glm::rotate(view_state.get_rotation().azimuth, glm::vec3(0, 1, 0)) *
           glm::translate(view_state.get_origin());
}

view_state ease(const view_state& from, const view_state& to, float speed) {
    return view_state{
            ease(from.get_origin(), to.get_origin(), speed),
            ease(from.get_eye_distance(), to.get_eye_distance(), speed),
            ease(from.get_rotation(), to.get_rotation(), speed)};
}


////////////////////////////////////////////////////////////////////////////////

void scene::set_visible_surface(std::experimental::optional<size_t> visible) {
    visible_surface_ = visible;
    visible_surface_changed_(get_visible_surface());
}

std::experimental::optional<size_t> scene::get_visible_surface() const {
    return visible_surface_;
}

void scene::set_visualise(bool visualise) {
    visualise_ = visualise;
    visualise_changed_(visualise);
}

bool scene::get_visualise() const { return visualise_; }

void scene::set_origin(const glm::vec3& origin) {
    view_state_.set_origin(origin);
    recompute_view_matrices();
    view_state_changed_(view_state_);
}

glm::vec3 scene::get_origin() const { return view_state_.get_origin(); }

void scene::set_eye_distance(float distance) {
    view_state_.set_eye_distance(distance);
    recompute_view_matrices();
    view_state_changed_(view_state_);
}

float scene::get_eye_distance() const { return view_state_.get_eye_distance(); }

void scene::set_rotation(const wayverb::core::az_el& az_el) {
    view_state_.set_rotation(az_el);
    recompute_view_matrices();
    view_state_changed_(view_state_);
}

wayverb::core::az_el scene::get_rotation() const {
    return view_state_.get_rotation();
}

view_state scene::get_view_state() const { return view_state_; }

void scene::set_viewport(const glm::vec2& viewport) {
    viewport_ = viewport;
    projection_matrix_ = compute_projection_matrix();
    inverse_projection_matrix_ = glm::inverse(projection_matrix_);
    projection_matrix_changed_(get_projection_matrix());
}

glm::vec2 scene::get_viewport() const { return viewport_; }

float scene::get_aspect() const { return viewport_.x / viewport_.y; }

float scene::get_item_radius() const { return item_radius; }

glm::mat4 scene::get_projection_matrix() const { return projection_matrix_; }
glm::mat4 scene::get_inverse_projection_matrix() const {
    return inverse_projection_matrix_;
}

glm::mat4 scene::compute_projection_matrix() const {
    return glm::tweakedInfinitePerspective(45.0f, get_aspect(), 0.01f);
}

void scene::recompute_view_matrices() {
    view_matrix_ = get_view_matrix(view_state_);
    inverse_view_matrix_ = glm::inverse(view_matrix_);
}

glm::vec3 scene::compute_world_camera_position() const {
    return inverse_view_matrix_[3];
}

glm::vec3 scene::compute_world_camera_direction() const {
    return glm::normalize(
            glm::vec3{inverse_view_matrix_ * glm::vec4{0, 0, -1, 0}});
}

glm::vec3 scene::compute_world_mouse_direction(const glm::vec2& pos) const {
    const auto viewport = get_viewport();
    const auto ray_clip = glm::vec4{
            (2 * pos.x) / viewport.x - 1, 1 - (2 * pos.y) / viewport.y, -1, 1};
    auto ray_eye = get_inverse_projection_matrix() * ray_clip;
    ray_eye = glm::vec4{ray_eye.x, ray_eye.y, -1, 0};
    return glm::normalize(glm::vec3{inverse_view_matrix_ * ray_eye});
}

scene::visible_surface_changed::connection
scene::connect_visible_surface_changed(
        visible_surface_changed::callback_type callback) {
    return visible_surface_changed_.connect(std::move(callback));
}

scene::visualise_changed::connection scene::connect_visualise_changed(
        visualise_changed::callback_type callback) {
    return visualise_changed_.connect(std::move(callback));
}

scene::view_state_changed::connection scene::connect_view_state_changed(
        view_state_changed::callback_type callback) {
    return view_state_changed_.connect(std::move(callback));
}

scene::projection_matrix_changed::connection
scene::connect_projection_matrix_changed(
        projection_matrix_changed::callback_type callback) {
    return projection_matrix_changed_.connect(std::move(callback));
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
