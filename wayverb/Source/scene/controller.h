#pragma once

#include "combined/model/app.h"

#include "raytracer/cl/reflection.h"

#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"

#include "glm/glm.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

namespace scene {

/// Should know about the model, and manipulate it correctly.
/// Here, 'model' means model::controller and also wayverb::combined::model::app
class controller final {
public:
    controller(wayverb::combined::model::app& app);
    ~controller() noexcept;

    void enablement_changed(bool enabled);

    void mouse_move(const MouseEvent& e);
    void mouse_down(const MouseEvent& e);
    void mouse_drag(const MouseEvent& e);
    void mouse_up(const MouseEvent& e);
    void mouse_wheel_move(const MouseEvent& e, const MouseWheelDetails& d);

    class mouse_action;

private:
    std::unique_ptr<mouse_action> start_action(const MouseEvent& e);

    template <typename It>
    auto get_hovered(It beg, It end, const glm::vec2& mouse_pos) const {
        using value_type = std::decay_t<decltype(beg->get_shared_ptr())>;

        const auto origin = app_.scene.compute_world_camera_position();
        const auto direction =
                app_.scene.compute_world_mouse_direction(mouse_pos);

        struct intersection {
            value_type it;
            double distance = 0;
        };

        intersection inter{};
        for (; beg != end; ++beg) {
            const auto diff = origin - (*beg)->get_position();
            const auto b = glm::dot(direction, diff);
            const auto c = glm::dot(diff, diff) -
                           glm::pow(app_.scene.get_item_radius(), 2);
            const auto det = glm::pow(b, 2) - c;
            if (0 <= det) {
                const auto sq_det = std::sqrt(det);
                const auto dist = std::min(-b + sq_det, -b - sq_det);
                if (!inter.it || dist < inter.distance) {
                    inter = intersection{beg->get_shared_ptr(), dist};
                }
            }
        }

        return inter;
    }

    template <typename T>
    auto do_action_with_closest_thing(
            const glm::vec2& mouse_pos,
            wayverb::combined::model::sources& sources,
            wayverb::combined::model::receivers& receivers,
            T&& action) const {
        const auto get_hovered_in_range = [this, mouse_pos](auto& range) {
            return get_hovered(std::begin(range), std::end(range), mouse_pos);
        };
        const auto hovered_source = get_hovered_in_range(sources);
        const auto hovered_receiver = get_hovered_in_range(receivers);

        using return_type = std::decay_t<decltype(action(hovered_source.it))>;

        if (hovered_source.it && hovered_receiver.it) {
            const auto source_is_closer =
                    hovered_source.distance < hovered_receiver.distance;

            if (source_is_closer) {
                return action(hovered_source.it);
            }

            return action(hovered_receiver.it);
        }

        if (hovered_source.it) {
            return action(hovered_source.it);
        }

        if (hovered_receiver.it) {
            return action(hovered_receiver.it);
        }

        return return_type{};
    }

    wayverb::combined::model::app& app_;

    std::unique_ptr<mouse_action> mouse_action_;

    bool allow_edit_ = true;
};

}  // namespace scene
