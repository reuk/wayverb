#pragma once

#include "core/az_el.h"

#include "utilities/event.h"

#include "glm/glm.hpp"

#include <experimental/optional>

namespace wayverb {
namespace combined {
namespace model {

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
    double get_eye_distance() const;

    void set_rotation(const wayverb::core::az_el& az_el);
    wayverb::core::az_el get_rotation() const;

    //  display stuff - not sure if this should really be here...

    void set_viewport(const glm::vec2& viewport);
    glm::vec2 get_viewport() const;
    float get_aspect() const;

    float get_item_radius() const;

    //  query actions

    glm::mat4 get_view_matrix() const;
    glm::mat4 get_inverse_view_matrix() const;

    glm::mat4 get_projection_matrix() const;
    glm::mat4 get_inverse_projection_matrix() const;

    glm::mat4 get_view_projection_matrix() const;

    glm::vec3 compute_world_camera_position() const;
    glm::vec3 compute_world_camera_direction() const;
    glm::vec3 compute_world_mouse_direction() const;

    using visible_surface_changed =
            util::event<std::experimental::optional<size_t>>;
    visible_surface_changed::connection connect_visible_surface_changed(
            visible_surface_changed::callback_type callback);

    using visualise_changed = util::event<bool>;
    visualise_changed::connection connect_visualise_changed(
            visualise_changed::callback_type callback);

    using view_matrix_changed = util::event<glm::mat4>;
    view_matrix_changed::connection connect_view_matrix_changed(
            view_matrix_changed::callback_type callback);

    using projection_matrix_changed = util::event<glm::mat4>;
    projection_matrix_changed::connection connect_projection_matrix_changed(
            projection_matrix_changed::callback_type callback);

private:
    glm::mat4 compute_view_matrix() const;
    glm::mat4 compute_projection_matrix() const;

    void recompute_view_matrices();
    void recompute_projection_matrices();

    //  data

    std::experimental::optional<size_t> visible_surface_;

    bool visualise_ = true;

    glm::vec3 origin_{0, 0, 0};
    float eye_distance_ = 1;
    wayverb::core::az_el az_el_{0, 0};

    const float item_radius_{0.4};

    glm::vec2 viewport_;

    glm::mat4 view_matrix_;
    glm::mat4 inverse_view_matrix_;

    glm::mat4 projection_matrix_;
    glm::mat4 inverse_projection_matrix_;

    //  events

    visible_surface_changed visible_surface_changed_;
    visualise_changed visualise_changed_;
    view_matrix_changed view_matrix_changed_;
    projection_matrix_changed projection_matrix_changed_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
