#pragma once

#include "core/az_el.h"

#include "utilities/event.h"

#include "glm/glm.hpp"

#include <experimental/optional>

namespace wayverb {
namespace combined {
namespace model {

/// Holds all the stuff that defines how the view is rendered.
/// We'll hold a 'master' one in the model, and a secondary one in the
/// view that can be updated more regularly to ease/animate camera movements.
class view_state final {
public:
    explicit view_state(const glm::vec3& origin = {0, 0, 0},
                        float eye_distance = 1,
                        const wayverb::core::az_el& az_el = {0, 0});

    void set_origin(const glm::vec3 & origin);
    glm::vec3 get_origin() const;

    void set_eye_distance(float distance);
    float get_eye_distance() const;

    void set_rotation(const wayverb::core::az_el& az_el);
    wayverb::core::az_el get_rotation() const;

private:
    glm::vec3 origin_{0, 0, 0};
    float eye_distance_ = 1;
    wayverb::core::az_el az_el_{0, 0};
};

glm::mat4 get_view_matrix(const view_state& view_state);

template <typename T>
constexpr auto ease(const T& from, const T& to, float speed) {
    return from + (to - from) * speed;
}

view_state ease(const view_state& from, const view_state& to, float speed);

////////////////////////////////////////////////////////////////////////////////

/// This stuff won't get sent to file, so it's fine to keep it here I think.
class scene final {
public:
    //  interactive data actions

    void set_visible_surface(std::experimental::optional<size_t> visible);
    std::experimental::optional<size_t> get_visible_surface() const;

    void set_visualise(bool visualise);
    bool get_visualise() const;

    //  view actions - affect the modelview matrix

    void set_origin(const glm::vec3& origin);
    glm::vec3 get_origin() const;

    void set_eye_distance(float distance);
    float get_eye_distance() const;

    void set_rotation(const wayverb::core::az_el& az_el);
    wayverb::core::az_el get_rotation() const;

    view_state get_view_state() const;

    //  display stuff - not sure if this should really be here...

    void set_viewport(const glm::vec2& viewport);
    glm::vec2 get_viewport() const;
    float get_aspect() const;

    float get_item_radius() const;

    //  query actions

    glm::mat4 get_projection_matrix() const;
    glm::mat4 get_inverse_projection_matrix() const;

    glm::vec3 compute_world_camera_position() const;
    glm::vec3 compute_world_camera_direction() const;
    glm::vec3 compute_world_mouse_direction(const glm::vec2& pos) const;

    using visible_surface_changed =
            util::event<std::experimental::optional<size_t>>;
    visible_surface_changed::connection connect_visible_surface_changed(
            visible_surface_changed::callback_type callback);

    using visualise_changed = util::event<bool>;
    visualise_changed::connection connect_visualise_changed(
            visualise_changed::callback_type callback);

    using view_state_changed = util::event<view_state>;
    view_state_changed::connection connect_view_state_changed(
            view_state_changed::callback_type callback);

    using projection_matrix_changed = util::event<glm::mat4>;
    projection_matrix_changed::connection connect_projection_matrix_changed(
            projection_matrix_changed::callback_type callback);

private:
    static constexpr float item_radius = 0.4;

    glm::mat4 compute_projection_matrix() const;

    void recompute_view_matrices();

    //  data

    std::experimental::optional<size_t> visible_surface_;

    bool visualise_ = true;

    view_state view_state_;

    glm::vec2 viewport_;

    glm::mat4 view_matrix_;
    glm::mat4 inverse_view_matrix_;

    glm::mat4 projection_matrix_;
    glm::mat4 inverse_projection_matrix_;

    //  events

    visible_surface_changed visible_surface_changed_;
    visualise_changed visualise_changed_;
    view_state_changed view_state_changed_;
    projection_matrix_changed projection_matrix_changed_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
