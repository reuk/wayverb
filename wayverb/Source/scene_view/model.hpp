#pragma once

#include "combined/model/app.h"

#include "raytracer/cl/reflection.h"

#include "utilities/work_queue.h"

#include "core/az_el.h"
#include "core/gpu_scene_data.h"

#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include "glm/glm.hpp"

namespace view {

/// This is the actual opengl view.
class scene final : public mglu::drawable, public mglu::updatable {
public:
    //  Nodes.

    void set_node_positions(util::aligned::vector<glm::vec3> positions);
    void set_node_pressures(util::aligned::vector<float> pressures);
    void set_node_colours(util::aligned::vector<glm::vec3> colours);

    void set_nodes_visible(bool visible);

    //  Reflections.
    
    void set_reflections(util::aligned::vector<util::aligned::vector<
                                 wayverb::raytracer::reflection>> reflections,
                         const glm::vec3& source);
    void set_distance_travelled(double distance);

    void set_reflections_visible(bool visible);

    //  Scene/surfaces.

    void set_scene(wayverb::core::gpu_scene_data scene);

    void set_highlighted_surface(int surface);
    void set_emphasis_colour(const glm::vec3& colour);

    //  Sources/receivers.

    void set_sources(util::aligned::vector<glm::vec3> sources);
    void set_receivers(util::aligned::vector<glm::vec3> receivers);

    //  Drawing functionality. 

    void set_projection_view_matrix(const glm::mat4& matrix);

    void update(float dt) override;

private:
    void do_draw(const glm::mat4& model_matrix) const override;
    glm::mat4 get_local_model_matrix() const override;

    glm::mat4 projection_view_matrix_;
    bool nodes_visible_ = false;
    bool reflections_visible_ = false;
};

}  // namespace view

////////////////////////////////////////////////////////////////////////////////

namespace model {

/// This stuff won't get sent to file, so it's fine to keep it here I think.
class scene final : public wayverb::combined::model::basic_member<scene> {
public:
    //  interactive data actions

    void set_visible_surface(int surface);
    int get_visible_surface() const;

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

    glm::vec3 compute_world_camera_position() const;
    glm::vec3 compute_world_camera_direction() const;
    glm::vec3 compute_world_mouse_direction() const;

private:
    glm::mat4 compute_view_matrix() const;
    glm::mat4 compute_projection_matrix() const;

    void recompute_view_matrices();
    void recompute_projection_matrices();

    //  data

    int visible_surface_;

    glm::vec3 origin_;
    float eye_distance_;
    wayverb::core::az_el az_el_;

    const float item_radius_{0.4};

    glm::vec2 viewport_;

    glm::mat4 view_matrix_;
    glm::mat4 inverse_view_matrix_;

    glm::mat4 projection_matrix_;
    glm::mat4 inverse_projection_matrix_;
};

}  // namespace model

////////////////////////////////////////////////////////////////////////////////

namespace controller {

/// Should know about the model, and manipulate it correctly.
/// Here, 'model' means model::scene and also wayverb::combined::model::app
class scene final {
public:
    scene(model::scene& temporary,
          wayverb::combined::model::persistent& persistent);

    void mouse_down(const MouseEvent& e);
    void mouse_drag(const MouseEvent& e);
    void mouse_up(const MouseEvent& e);
    void mouse_wheel_move(const MouseEvent& e, const MouseWheelDetails& d);

    class mouse_action;
private:
    std::unique_ptr<mouse_action> start_action(const MouseEvent& e);

    glm::vec3 compute_world_mouse_direction(const glm::vec2& pos) const;

    template <typename It>
    auto get_hovered(It beg, It end, const glm::vec2& mouse_pos) {
        using value_type = std::decay_t<decltype(*beg)>;

        const auto origin = temporary_.compute_world_camera_position();
        const auto direction = compute_world_mouse_direction(mouse_pos);

        struct intersection {
            value_type* it;
            double distance;
        };

        intersection inter{nullptr, 0};
        for (; beg != end; ++beg) {
            const auto diff = origin - beg->position().get();
            const auto b = glm::dot(direction, diff);
            const auto c = glm::dot(diff, diff) -
                           glm::pow(temporary_.get_item_radius(), 2);
            const auto det = glm::pow(b, 2) - c;
            if (0 <= det) {
                const auto sq_det = std::sqrt(det);
                const auto dist = std::min(-b + sq_det, -b - sq_det);
                if (!inter.it || dist < inter.distance) {
                    inter = intersection{&(*beg), dist};
                }
            }
        }

        return inter;
    }

    model::scene& temporary_;
    wayverb::combined::model::persistent& persistent_;

    std::unique_ptr<mouse_action> mouse_action_;
};

}  // namespace controller
