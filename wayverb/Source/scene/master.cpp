#include "master.h"

#include "controller.h"
#include "view.h"

#include "Application.h"
#include "CommandIDs.h"

#include "../UtilityComponents/generic_renderer.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace scene {

class master::impl final : public Component {
public:
    impl(wayverb::combined::model::app& app)
            : app_{app}
            , controller_{app_} {
        //  If the model visible surface changes, update the view visible
        //  surface.
        app_.scene.connect_visible_surface_changed([&](auto visible) {
            view_.command([=](auto& r) { r.set_highlighted_surface(visible); });
        });

        //  If a model matrix changes, update the view matrix.
        app_.scene.connect_view_matrix_changed([&](auto matrix) {
            view_.command([=](auto& r) { r.set_view_matrix(matrix); });
        });

        app_.scene.connect_projection_matrix_changed([&](auto matrix) {
            view_.command([=](auto& r) { r.set_projection_matrix(matrix); });
        });

        //  Set up the scene model so that everything is visible.
        const auto scene_data = app_.project.get_scene_data();
        auto triangles = scene_data.get_triangles();
        auto vertices = util::map_to_vector(begin(scene_data.get_vertices()),
                                            end(scene_data.get_vertices()),
                                            wayverb::core::to_vec3{});

        const auto aabb = wayverb::core::geo::compute_aabb(vertices);
        const auto origin = -centre(aabb);
        const auto radius = glm::distance(aabb.get_min(), aabb.get_max()) / 2;

        app_.scene.set_origin(origin);
        app_.scene.set_eye_distance(2 * radius);
        app_.scene.set_rotation(wayverb::core::az_el{M_PI / 4, M_PI / 6});

        //  When view is initialised, set the scene correctly
        //  The callback will be run on the main message thread, so we can do
        //  what we like (within reason).
        view_.connect_context_created(
                [ this, t = std::move(triangles), v = std::move(vertices) ](
                        auto&) {
                    //  Get scene data in correct format.
                    //  This command will be run on the graphics thread, so it
                    //  must be thread safe.
                    view_.command([
                        t = std::move(t),
                        v = std::move(v),
                        vm = app_.scene.get_view_matrix(),
                        pm = app_.scene.get_projection_matrix()
                    ](auto& r) {
                        r.set_scene(t.data(), t.size(), v.data(), v.size());
                        r.set_view_matrix(vm);
                        r.set_projection_matrix(pm);
                        //  TODO we might need to set other state here too.
                    });
                });

        //  Hook up the engine.

        //  IMPORTANT app callbacks might be called from any thread - don't
        //  access unprotected common state inside engine callbacks!
        app_.connect_node_positions([this](auto descriptor) {
            view_.command([d = std::move(descriptor)](auto& renderer) {
                renderer.set_node_positions(
                        wayverb::waveguide::compute_node_positions(d));
            });
        });

        app_.connect_node_pressures([this](auto pressures, auto distance) {
            view_.command([ p = std::move(pressures),
                            d = distance ](auto& renderer) {
                renderer.set_node_colours(
                        util::map_to_vector(begin(p), end(p), [](auto p) {
                            const auto a = std::abs(p) * 1000.0;
                            return 0 < p ? glm::vec4{a, a, 0, a}
                                         : glm::vec4{0, 0, a, a};
                        }));
                renderer.set_distance_travelled(d);
            });
        });

        app_.connect_reflections([this](auto reflections, auto source) {
            view_.command(
                    [ r = std::move(reflections), s = source ](auto& renderer) {
                        renderer.set_reflections(std::move(r), s);
                    });
        });

        //  We want to catch mouse events and dispatch our own commands to the
        //  view, so we'll disable mouse events directly on the view.
        view_.setInterceptsMouseClicks(false, false);
        //  Make the view visible.
        addAndMakeVisible(view_);
    }

    void resized() override {
        app_.scene.set_viewport(glm::vec2{getWidth(), getHeight()});
        view_.setBounds(getLocalBounds());
    }

    void mouseDown(const MouseEvent& e) override { controller_.mouse_down(e); }

    void mouseDrag(const MouseEvent& e) override { controller_.mouse_drag(e); }

    void mouseUp(const MouseEvent& e) override { controller_.mouse_up(e); }

    void mouseWheelMove(const MouseEvent& e,
                        const MouseWheelDetails& d) override {
        controller_.mouse_wheel_move(e, d);
    }

private:
    //  We don't directly use the view, because it needs to run on its own gl
    //  thread.
    //  Instead, we wrap it in an object which supplies async command queues.
    generic_renderer<scene::view> view_;

    //  Keep a reference to the global model.
    wayverb::combined::model::app& app_;

    //  This object decides how to interpret user input, and updates the models
    //  as appropriate.
    scene::controller controller_;
};

master::master(wayverb::combined::model::app& app)
        : pimpl_{std::make_unique<impl>(app)} {
    set_help("model viewport",
             "This area displays the currently loaded 3D model. Click and drag "
             "to rotate the model, or use the mouse wheel to zoom in and out.");

    addAndMakeVisible(*pimpl_);
}

master::~master() noexcept = default;

void master::resized() { pimpl_->setBounds(getLocalBounds()); }

}  // namespace scene
