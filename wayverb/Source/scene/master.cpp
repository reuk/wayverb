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
            , controller_{app_}
            , visible_surface_changed_connection_{
                app.scene.connect_visible_surface_changed([this](auto visible) {
                    view_.command([=](auto& r) { r.set_highlighted_surface(visible); });
                })}
            , view_state_changed_connection_{
                app.scene.connect_view_state_changed([this](auto state) {
                    view_.command([=](auto& r) { r.set_view_state(state); });
                })}
            , projection_matrix_changed_connection_{
                app_.scene.connect_projection_matrix_changed([this](auto matrix) {
                    view_.command([=](auto& r) { r.set_projection_matrix(matrix); });
                })}
            , visualise_changed_connection_{
                 app_.scene.connect_visualise_changed([this](auto should_visualise) {
                    if (should_visualise) {
                        //  IMPORTANT app callbacks might be called from any thread -
                        //  don't access unprotected common state inside engine
                        //  callbacks!
                        positions_changed_ = wayverb::combined::
                                waveguide_node_positions_changed::scoped_connection{
                                        app_.connect_node_positions([this](
                                                auto descriptor) {
                                            view_.command([d = std::move(descriptor)](
                                                    auto& renderer) {
                                                renderer.set_node_positions(
                                                        wayverb::waveguide::
                                                                compute_node_positions(
                                                                        d));
                                            });
                                        })};

                        pressures_changed_ = wayverb::combined::
                                waveguide_node_pressures_changed::scoped_connection{
                                        app_.connect_node_pressures([this](
                                                auto pressures, auto distance) {
                                            view_.command([
                                                p = std::move(pressures),
                                                d = distance
                                            ](auto& renderer) {
                                                renderer.set_node_pressures(std::move(p));
                                                renderer.set_distance_travelled(d);
                                            });
                                        })};

                        reflections_generated_ =
                                wayverb::combined::raytracer_reflections_generated::
                                        scoped_connection{app_.connect_reflections(
                                                [this](auto reflections, auto source) {
                                                    view_.command([
                                                        r = std::move(reflections),
                                                        s = source
                                                    ](auto& renderer) {
                                                        renderer.set_reflections(
                                                                std::move(r), s);
                                                    });
                                                })};

                    } else {
                        positions_changed_ = wayverb::combined::
                                waveguide_node_positions_changed::scoped_connection{};
                        pressures_changed_ = wayverb::combined::
                                waveguide_node_pressures_changed::scoped_connection{};
                        reflections_generated_ = wayverb::combined::
                                raytracer_reflections_generated::scoped_connection{};
                    }
                })}
            , finished_connection_{
                app_.connect_finished([this] {
                    queue_.push([this] {
                        view_.command([](auto& renderer) { renderer.clear(); });
                    });
                })}
            , sources_connection_{
                //  When sources or receivers change, update the view.
                //  Assume model updates come from the message thread.
                app_.project.persistent.sources()->connect([this](auto& sources) {
                    auto copy = sources;
                    view_.command([r = std::move(copy)](auto& renderer) {
                        renderer.set_sources(std::move(r));
                    });
                })}
            , receivers_connection_{
                app_.project.persistent.receivers()->connect([this](auto& receivers) {
                    auto copy = receivers;
                    view_.command([r = std::move(copy)](auto& renderer) {
                        renderer.set_receivers(std::move(r));
                    });
                })}
            {
        //  Set up the scene model so that everything is visible.
        const auto scene_data = app_.project.get_scene_data();
        auto triangles = scene_data.get_triangles();
        auto vertices = util::map_to_vector(begin(scene_data.get_vertices()),
                                            end(scene_data.get_vertices()),
                                            wayverb::core::to_vec3{});

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
                        vs = app_.scene.get_view_state(),
                        pm = app_.scene.get_projection_matrix()
                    ](auto& r) {
                        r.set_scene(t.data(), t.size(), v.data(), v.size());
                        r.set_view_state(vs);
                        r.set_projection_matrix(pm);
                        //  TODO we might need to set other state here too.
                    });
                });

        app_.project.persistent.sources()->notify();
        app_.project.persistent.receivers()->notify();

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

    void mouseMove(const MouseEvent& e) override { controller_.mouse_move(e); }

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
    async_work_queue queue_;

    //  This object decides how to interpret user input, and updates the models
    //  as appropriate.
    scene::controller controller_;

    wayverb::combined::model::scene::visible_surface_changed::scoped_connection
            visible_surface_changed_connection_;
    wayverb::combined::model::scene::view_state_changed::scoped_connection
            view_state_changed_connection_;
    wayverb::combined::model::scene::projection_matrix_changed::
            scoped_connection projection_matrix_changed_connection_;
    wayverb::combined::model::scene::visualise_changed::scoped_connection
            visualise_changed_connection_;
    wayverb::combined::model::app::finished::scoped_connection
            finished_connection_;
    wayverb::combined::model::sources::scoped_connection sources_connection_;
    wayverb::combined::model::receivers::scoped_connection
            receivers_connection_;

    wayverb::combined::waveguide_node_positions_changed::scoped_connection
            positions_changed_;
    wayverb::combined::waveguide_node_pressures_changed::scoped_connection
            pressures_changed_;
    wayverb::combined::raytracer_reflections_generated::scoped_connection
            reflections_generated_;
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
