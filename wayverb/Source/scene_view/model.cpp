#include "model.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/noise.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

namespace view {

void scene::set_node_positions(util::aligned::vector<glm::vec3> positions) {}

void scene::set_node_pressures(util::aligned::vector<float> pressures) {}

void scene::set_node_colours(util::aligned::vector<glm::vec3> colours) {}

void scene::set_nodes_visible(bool visible) {
    nodes_visible_ = visible;
}

void scene::show_nodes() {}

void scene::hide_nodes() {}

void scene::set_reflections(
        util::aligned::vector<util::aligned::vector<
                wayverb::raytracer::reflection>> reflections,
        const glm::vec3& source) {}

void scene::set_distance_travelled(double distance) {}

void scene::set_reflections_visible(bool visible) {
    reflections_visible_ = visible;
}

void scene::set_scene(wayverb::core::gpu_scene_data scene) {}

void scene::set_highlighted_surface(int surface) {}

void scene::set_emphasis_colour(const glm::vec3& colour) {}

void scene::set_sources(util::aligned::vector<glm::vec3> sources) {}

void scene::set_receivers(util::aligned::vector<glm::vec3> receivers) {}

void scene::set_projection_view_matrix(const glm::mat4& matrix) {
    projection_view_matrix_ = matrix;
}

void scene::update(float dt) {}

void scene::do_draw(const glm::mat4& modelview_matrix) const {
    const auto c = 0.0;
    glClearColor(c, c, c, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //  TODO Set up shaders.

    //  TODO Draw objects.
}

glm::mat4 scene::get_local_modelview_matrix() const { return glm::mat4{}; }

}  // namespace view

////////////////////////////////////////////////////////////////////////////////

namespace model {

void scene::set_visible_surface(int surface) {
    visible_surface_ = surface;
    notify();
}

int scene::get_visible_surface() const { return visible_surface_; }

void scene::set_origin(const glm::vec3& origin) {
    origin_ = origin;
    recompute_view_matrices();
    notify();
}

glm::vec3 scene::get_origin() const { return origin_; }

void scene::set_eye_distance(float distance) {
    eye_distance_ = std::max(0.0f, distance);
    recompute_view_matrices();
    notify();
}

double scene::get_eye_distance() const { return eye_distance_; }

void scene::set_rotation(const wayverb::core::az_el& az_el) {
    az_el_ = wayverb::core::az_el{
            az_el.azimuth,
            clamp(az_el.elevation, util::make_range(-M_PI / 2, M_PI / 2))};
    recompute_view_matrices();
    notify();
}

wayverb::core::az_el scene::get_rotation() const { return az_el_; }

void scene::set_viewport(const glm::vec2& viewport) {
    viewport_ = viewport;
    recompute_projection_matrices();
    notify();
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
    return glm::perspective(45.0f, get_aspect(), 0.05f, 1000.0f);
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
}

void scene::recompute_projection_matrices() {
    projection_matrix_ = compute_projection_matrix();
    inverse_projection_matrix_ = glm::inverse(projection_matrix_);
}

glm::vec3 scene::compute_world_camera_position() const {
    return inverse_view_matrix_[3];
}

glm::vec3 scene::compute_world_camera_direction() const {
    return glm::normalize(
            glm::vec3{inverse_view_matrix_ * glm::vec4{0, 0, -1, 0}});
}

}  // namespace model

////////////////////////////////////////////////////////////////////////////////

namespace controller {

/// A mouse action is a series of movements while a mouse button is held.
class scene::mouse_action {
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

class rotate_action final : public scene::mouse_action {
public:
    rotate_action(model::scene& scene)
            : scene_{scene}
            , initial_{scene_.get_rotation()} {}

    void mouse_drag(const MouseEvent& e) override {
        const auto diff = e.getOffsetFromDragStart();
        const auto angle_scale = 0.01;
        scene_.set_rotation(wayverb::core::az_el{
                static_cast<float>(diff.x * angle_scale + initial_.azimuth),
                static_cast<float>(diff.y * angle_scale + initial_.elevation)});
    }

    void mouse_wheel_move(const MouseEvent&,
                          const MouseWheelDetails& d) override {
        scene_.set_eye_distance(scene_.get_eye_distance() + d.deltaY);
    }

    void mouse_up(const MouseEvent&) override {}

private:
    model::scene& scene_;
    wayverb::core::az_el initial_;
};

class pan_action final : public scene::mouse_action {
public:
    pan_action(model::scene& scene)
            : scene_{scene} {}

    void mouse_drag(const MouseEvent&) override {
        //  TODO pan
        //  TODO move origin in the plane of the current orientation
    }

    void mouse_wheel_move(const MouseEvent&,
                          const MouseWheelDetails& d) override {
        scene_.set_eye_distance(scene_.get_eye_distance() + d.deltaY);
    }

    void mouse_up(const MouseEvent&) override {}

private:
    model::scene& scene_;
};

template <typename Item>
class move_item_action final : public scene::mouse_action {
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

scene::scene(model::scene& temporary,
             wayverb::combined::model::persistent& persistent)
        : temporary_{temporary}
        , persistent_{persistent} {}

void scene::mouse_down(const MouseEvent& e) { mouse_action_ = start_action(e); }

void scene::mouse_drag(const MouseEvent& e) {
    //  Forward the position to the currently engaged action.
    if (mouse_action_) {
        mouse_action_->mouse_drag(e);
    }
}

void scene::mouse_up(const MouseEvent& e) {
    //  If a mouse button is released, quit the current action.
    //  If there are still buttons held, start the appropriate action for the
    //  remaining buttons.
    if (mouse_action_) {
        mouse_action_->mouse_up(e);
        mouse_action_ = nullptr;
    }
    mouse_action_ = start_action(e);
}

void scene::mouse_wheel_move(const MouseEvent& e, const MouseWheelDetails& d) {
    if (mouse_action_) {
        mouse_action_->mouse_wheel_move(e, d);
    }
}

std::unique_ptr<scene::mouse_action> scene::start_action(const MouseEvent& e) {
    //  If middle and left mouse buttons are pressed, start panning action.
    if (e.mods.isLeftButtonDown() && e.mods.isRightButtonDown()) {
        return std::make_unique<pan_action>(temporary_);
    }

    //  If left mouse button is pressed, check for hovered items.
    //  If any, start move action with hovered item.
    //  Else, do nothing.
    if (e.mods.isLeftButtonDown()) {
        const auto pos = wayverb::core::to_vec2{}(e.getPosition());
        const auto hovered_source =
                get_hovered(std::begin(persistent_.sources()),
                            std::end(persistent_.sources()),
                            pos);
        const auto hovered_receiver =
                get_hovered(std::begin(persistent_.receivers()),
                            std::end(persistent_.receivers()),
                            pos);

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

    //  If middle mouse button is pressed, start rotation action.
    if (e.mods.isRightButtonDown()) {
        return std::make_unique<rotate_action>(temporary_);
    }

    //  If nothing (important) is pressed then do nothing.
    return nullptr;
}

glm::vec3 scene::compute_world_mouse_direction(const glm::vec2& pos) const {
    const auto ray_clip =
            glm::vec4{(2 * pos.x) / temporary_.get_viewport().x - 1,
                      1 - (2 * pos.y) / temporary_.get_viewport().y,
                      -1,
                      1};
    auto ray_eye = temporary_.get_inverse_projection_matrix() * ray_clip;
    ray_eye = glm::vec4{ray_eye.x, ray_eye.y, -1, 0};
    return glm::normalize(
            glm::vec3{temporary_.get_inverse_view_matrix() * ray_eye});
}

}  // namespace controller
