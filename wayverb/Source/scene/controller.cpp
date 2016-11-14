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
};

class rotate_action final : public controller::mouse_action {
public:
    rotate_action(wayverb::combined::model::scene& model)
            : model_{model}
            , initial_{model_.get_rotation()} {}

    void mouse_drag(const MouseEvent& e) override {
        const auto diff = e.getOffsetFromDragStart();
        const auto angle_scale = 0.01;
        model_.set_rotation(wayverb::core::az_el{
                static_cast<float>(diff.x * angle_scale + initial_.azimuth),
                static_cast<float>(diff.y * angle_scale + initial_.elevation)});
    }

private:
    wayverb::combined::model::scene& model_;
    wayverb::core::az_el initial_;
};

class pan_action final : public controller::mouse_action {
public:
    pan_action(wayverb::combined::model::scene& model)
            : model_{model}
            , camera_position_{model_.compute_world_camera_position()}
            , camera_direction_{model_.compute_world_camera_direction()}
            , camera_distance_{model_.get_eye_distance()}
            , initial_position_{model_.get_origin()} {}

    void mouse_drag(const MouseEvent& e) override {
        //  Find mouse position on plane perpendicular to camera direction
        //  through the rotation origin.
        const auto initial = compute_world_mouse_position(glm::vec2{
                e.getMouseDownPosition().x, e.getMouseDownPosition().y});
        const auto current = compute_world_mouse_position(
                glm::vec2{e.getPosition().x, e.getPosition().y});
        //  Adjust the model origin based on the difference between the initial
        //  and current mouse positions.
        model_.set_origin(initial_position_ + current - initial);
    }

private:
    glm::vec3 compute_world_mouse_position(const glm::vec2& mouse_pos) {
        const auto mouse_direction =
                model_.compute_world_mouse_direction(mouse_pos);
        return mouse_direction * camera_distance_ /
               glm::dot(camera_direction_, mouse_direction);
    }

    wayverb::combined::model::scene& model_;
    glm::vec3 camera_position_;
    glm::vec3 camera_direction_;
    float camera_distance_;
    glm::vec3 initial_position_;
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

    ~move_item_action() noexcept {
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

void controller::mouse_move(const MouseEvent& e) {
    std::cout << "mouse move\n";

    //  We do things like this to minimize redraw requests.

    //  Make a copy of sources and receivers.
    auto sources = app_.project.persistent.sources();
    auto receivers = app_.project.persistent.receivers();

    //  Ensure nothing is hovered.
    for (auto& i : sources) {
        i.hover_state().set_hovered(false);
    }
    for (auto& i : receivers) {
        i.hover_state().set_hovered(false);
    }
    
    //  look for hovered items, notify if something is hovered.
    do_action_with_closest_thing(wayverb::core::to_vec2{}(e.getPosition()),
        sources,
        receivers,
        [](auto& i) {
            i.hover_state().set_hovered(true);
            return 1;
        });

    //  Copy sources and receivers back to master model.
    app_.project.persistent.sources() = sources;
    app_.project.persistent.receivers() = receivers;
}

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
        mouse_action_ = nullptr;
    }
}

void controller::mouse_wheel_move(const MouseEvent& e,
                                  const MouseWheelDetails& d) {
    //  Only zoom if another action is not ongoing.
    if (mouse_action_ == nullptr) {
        const auto current_distance = app_.scene.get_eye_distance();
        const auto diff = current_distance * d.deltaY;
        app_.scene.set_eye_distance(current_distance + diff);
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
        return do_action_with_closest_thing(wayverb::core::to_vec2{}(e.getPosition()),
            app_.project.persistent.sources(),
            app_.project.persistent.receivers(),
                [](const auto& i) -> std::unique_ptr<mouse_action> {
                    return make_move_item_action_ptr(i);
                });
    }

    //  If nothing (important) is pressed then do nothing.
    return nullptr;
}

}  // namespace scene
