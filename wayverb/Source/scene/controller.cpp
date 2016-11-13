#include "controller.h"

namespace scene {

/// A mouse action is a series of movements while a mouse button is held.
class controller::mouse_action {
public:
    /// Construct on mouse-down.
    mouse_action() = default;

    mouse_action(const mouse_action&) = default;
    mouse_action(mouse_action&&) noexcept = default;

    mouse_action& operator=(const mouse_action&) = default;
    mouse_action& operator=(mouse_action&&) noexcept = default;

    /// Destroy on mouse-up.
    virtual ~mouse_action() noexcept = default;

    /// Do something when the mouse is moved.
    virtual void mouse_drag(const MouseEvent& e) = 0;

    /// Do something when the mouse is scrolled.
    virtual void mouse_wheel_move(const MouseEvent& e,
                                  const MouseWheelDetails& d) = 0;

    /// Sent just before object is destroyed.
    virtual void mouse_up(const MouseEvent& e) = 0;
};

class rotate_action final : public controller::mouse_action {
public:
    rotate_action(wayverb::combined::model::scene& controller)
            : controller_{controller}
            , initial_{controller_.get_rotation()} {}

    void mouse_drag(const MouseEvent& e) override {
        const auto diff = e.getOffsetFromDragStart();
        const auto angle_scale = 0.01;
        controller_.set_rotation(wayverb::core::az_el{
                static_cast<float>(diff.x * angle_scale + initial_.azimuth),
                static_cast<float>(diff.y * angle_scale + initial_.elevation)});
    }

    void mouse_wheel_move(const MouseEvent&,
                          const MouseWheelDetails& d) override {
        controller_.set_eye_distance(controller_.get_eye_distance() + d.deltaY);
    }

    void mouse_up(const MouseEvent&) override {}

private:
    wayverb::combined::model::scene& controller_;
    wayverb::core::az_el initial_;
};

class pan_action final : public controller::mouse_action {
public:
    pan_action(wayverb::combined::model::scene& controller)
            : controller_{controller} {}

    void mouse_drag(const MouseEvent&) override {
        //  TODO pan
        //  TODO move origin in the plane of the current orientation
    }

    void mouse_wheel_move(const MouseEvent&,
                          const MouseWheelDetails& d) override {
        controller_.set_eye_distance(controller_.get_eye_distance() + d.deltaY);
    }

    void mouse_up(const MouseEvent&) override {}

private:
    wayverb::combined::model::scene& controller_;
};

template <typename Item>
class move_item_action final : public controller::mouse_action {
public:
    move_item_action(Item& item)
            : item_{item}
            , initial_{item.position().get()} {}

    void mouse_drag(const MouseEvent&) override {
        //  TODO highlight item
        //  TODO move item
    }

    void mouse_wheel_move(const MouseEvent&,
                          const MouseWheelDetails&) override {}

    void mouse_up(const MouseEvent&) override {
        //  TODO remove highlighting
    }

private:
    Item& item_;
    glm::vec3 initial_;
};

template <typename T>
auto make_move_item_action_ptr(T& t) {
    return std::make_unique<move_item_action<T>>(t);
}

controller::controller(wayverb::combined::model::app& app)
        : app_{app} {}

controller::~controller() noexcept = default;

void controller::mouse_down(const MouseEvent& e) {
    mouse_action_ = start_action(e);
}

void controller::mouse_drag(const MouseEvent& e) {
    //  Forward the position to the currently engaged action.
    if (mouse_action_) {
        mouse_action_->mouse_drag(e);
    }
}

void controller::mouse_up(const MouseEvent& e) {
    //  If a mouse button is released, quit the current action.
    //  If there are still buttons held, start the appropriate action for the
    //  remaining buttons.
    if (mouse_action_) {
        mouse_action_->mouse_up(e);
        mouse_action_ = nullptr;
    }
    mouse_action_ = start_action(e);
}

void controller::mouse_wheel_move(const MouseEvent& e,
                                  const MouseWheelDetails& d) {
    if (mouse_action_) {
        mouse_action_->mouse_wheel_move(e, d);
    }
}

std::unique_ptr<controller::mouse_action> controller::start_action(
        const MouseEvent& e) {
    //  If middle and left mouse buttons are pressed, start panning action.
    if (e.mods.isMiddleButtonDown()) {
        return std::make_unique<pan_action>(app_.scene);
    }

    //  If middle mouse button is pressed, start rotation action.
    if (e.mods.isRightButtonDown()) {
        return std::make_unique<rotate_action>(app_.scene);
    }

    //  If left mouse button is pressed, check for hovered items.
    //  If any, start move action with hovered item.
    //  Else, do nothing.
    if (e.mods.isLeftButtonDown()) {
        const auto pos = wayverb::core::to_vec2{}(e.getPosition());
        const auto get_hovered_in_range = [this, pos](auto& range) {
            return get_hovered(std::begin(range), std::end(range), pos);
        };
        const auto hovered_source =
                get_hovered_in_range(app_.project.persistent.sources());
        const auto hovered_receiver =
                get_hovered_in_range(app_.project.persistent.receivers());

        if (hovered_source.it != nullptr && hovered_receiver.it != nullptr) {
            const auto source_is_closer =
                    hovered_source.distance < hovered_receiver.distance;

            if (source_is_closer) {
                return make_move_item_action_ptr(*hovered_source.it);
            }

            return make_move_item_action_ptr(*hovered_receiver.it);
        }

        if (hovered_source.it != nullptr) {
            return make_move_item_action_ptr(*hovered_source.it);
        }

        if (hovered_receiver.it != nullptr) {
            return make_move_item_action_ptr(*hovered_receiver.it);
        }

        return nullptr;
    }

    //  If nothing (important) is pressed then do nothing.
    return nullptr;
}

glm::vec3 controller::compute_world_mouse_direction(
        const glm::vec2& pos) const {
    const auto viewport = app_.scene.get_viewport();
    const auto ray_clip = glm::vec4{
            (2 * pos.x) / viewport.x - 1, 1 - (2 * pos.y) / viewport.y, -1, 1};
    auto ray_eye = app_.scene.get_inverse_projection_matrix() * ray_clip;
    ray_eye = glm::vec4{ray_eye.x, ray_eye.y, -1, 0};
    return glm::normalize(
            glm::vec3{app_.scene.get_inverse_view_matrix() * ray_eye});
}

}  // namespace scene
