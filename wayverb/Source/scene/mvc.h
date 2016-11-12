#pragma once

#include "combined/model/app.h"

#include "raytracer/cl/reflection.h"

#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"

#include "glm/glm.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

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

    void set_view_projection_matrix(const glm::mat4& matrix);

    void update(float dt) override;

private:
    void do_draw(const glm::mat4& model_matrix) const override;
    glm::mat4 get_local_model_matrix() const override;

    glm::mat4 view_projection_matrix_;
    bool nodes_visible_ = false;
    bool reflections_visible_ = false;
};

}  // namespace view

////////////////////////////////////////////////////////////////////////////////

namespace controller {

/// Should know about the model, and manipulate it correctly.
/// Here, 'model' means model::scene and also wayverb::combined::model::app
class scene final {
public:
    scene(wayverb::combined::model::app& app);
    ~scene() noexcept;

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

        const auto origin = app_.scene.compute_world_camera_position();
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
                           glm::pow(app_.scene.get_item_radius(), 2);
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

    wayverb::combined::model::app& app_;

    std::unique_ptr<mouse_action> mouse_action_;
};

}  // namespace controller
