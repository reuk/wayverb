#include "master.h"

#include "main_model.h"

#include "controller.h"
#include "view.h"

#include "AngularLookAndFeel.h"
#include "Application.h"
#include "CommandIDs.h"

#include "../UtilityComponents/generic_renderer.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace scene {

class master::impl final : public Component, public generic_renderer<view>::Listener, public SettableTooltipClient {
public:
    impl(main_model& model)
            : model_{model}
            , controller_{model_}
            //  These events will come from the message thread.
            , visible_surface_changed_connection_{
                model_.scene.connect_visible_surface_changed([this](auto visible) {
                    view_.high_priority_command([=](auto& r) { r.set_highlighted_surface(visible); });
                })}
            , view_state_changed_connection_{
                model_.scene.connect_view_state_changed([this](auto state) {
                    view_.high_priority_command([=](auto& r) { r.set_view_state(state); });
                })}
            , projection_matrix_changed_connection_{
                model_.scene.connect_projection_matrix_changed([this](auto matrix) {
                    view_.high_priority_command([=](auto& r) { r.set_projection_matrix(matrix); });
                })}
            //  These events come from the engine, so they must be queued onto the message thread.
            , visualise_changed_connection_{
                 model_.scene.connect_visualise_changed([this](auto should_visualise) {
                    if (should_visualise) {
                        //  IMPORTANT app callbacks might be called from any thread -
                        //  don't access unprotected common state inside engine
                        //  callbacks!
                        positions_changed_ = main_model::waveguide_node_positions_changed::scoped_connection{
                                        model_.connect_node_positions([this](
                                                auto descriptor) {
                                            view_.high_priority_command([d = std::move(descriptor)](
                                                    auto& renderer) {
                                                renderer.set_node_positions(
                                                        wayverb::waveguide::
                                                                compute_node_positions(
                                                                        d));
                                            });
                                        })};

                        pressures_changed_ = main_model::waveguide_node_pressures_changed::scoped_connection{
                                        model_.connect_node_pressures([this](
                                                auto pressures, auto distance) {
                                            view_.low_priority_command([
                                                p = std::move(pressures),
                                                d = distance
                                            ](auto& renderer) {
                                                renderer.set_node_pressures(std::move(p));
                                                renderer.set_distance_travelled(d);
                                            });
                                        })};

                        reflections_generated_ =
                                main_model::raytracer_reflections_generated::scoped_connection{model_.connect_reflections(
                                                [this](auto reflections, auto source) {
                                                    view_.high_priority_command([
                                                        r = std::move(reflections),
                                                        s = source
                                                    ](auto& renderer) {
                                                        renderer.set_reflections(
                                                                std::move(r), s);
                                                    });
                                                })};

                    } else {
                        positions_changed_     = main_model::waveguide_node_positions_changed::scoped_connection{};
                        pressures_changed_     = main_model::waveguide_node_pressures_changed::scoped_connection{};
                        reflections_generated_ = main_model::raytracer_reflections_generated::scoped_connection{};
                        view_.high_priority_command([](auto& renderer) { renderer.clear(); });
                    }
                })}
            , sources_connection_{
                //  When sources or receivers change, update the view.
                //  Assume model updates come from the message thread.
                model_.project.persistent.sources()->connect([this](auto& sources) {
                    auto copy = sources;
                    view_.high_priority_command([r = std::move(copy)](auto& renderer) {
                        renderer.set_sources(std::move(r));
                    });
                })}
            , receivers_connection_{
                model_.project.persistent.receivers()->connect([this](auto& receivers) {
                    auto copy = receivers;
                    view_.high_priority_command([r = std::move(copy)](auto& renderer) {
                        renderer.set_receivers(std::move(r));
                    });
                })}
            , begun_{model_.connect_begun([this] {
                    setEnabled(false);
                })}
            , finished_{model_.connect_finished([this] {
                    setEnabled(true);
                    view_.high_priority_command([](auto& renderer) { renderer.clear(); });
                })}
            {
        model_.project.persistent.sources()->notify();
        model_.project.persistent.receivers()->notify();

        //  We want to catch mouse events and dispatch our own commands to the
        //  view, so we'll disable mouse events directly on the view.
        view_.setInterceptsMouseClicks(false, false);

        //  Make the view visible.
        addAndMakeVisible(view_);

        setTooltip("left-click: move sources and receivers\n"
               "right-click: rotate view\n"
               "middle-click: pan view\n"
               "scroll: zoom view");
    }

    void resized() override {
        model_.scene.set_viewport(glm::vec2{getWidth(), getHeight()});
        view_.setBounds(getLocalBounds());
    }

    //  Where 'enablement' is the ability of the component to modify the model.
    //  View updates will still happen if disabled, as they don't affect the
    //  persistent-data part of the model.
    void enablementChanged() override {
        controller_.enablement_changed(isEnabled());
    }

    void mouseMove(const MouseEvent& e) override { controller_.mouse_move(e); }

    void mouseDown(const MouseEvent& e) override { controller_.mouse_down(e); }

    void mouseDrag(const MouseEvent& e) override { controller_.mouse_drag(e); }

    void mouseUp(const MouseEvent& e) override { controller_.mouse_up(e); }

    void mouseWheelMove(const MouseEvent& e,
                        const MouseWheelDetails& d) override {
        controller_.mouse_wheel_move(e, d);
    }

    void context_created(generic_renderer<scene::view>&) override {
        //  Set up the scene model so that everything is visible.
        const auto scene_data = model_.project.get_scene_data();
        auto triangles = scene_data.get_triangles();
        auto vertices = util::map_to_vector(begin(scene_data.get_vertices()),
                                            end(scene_data.get_vertices()),
                                            wayverb::core::to_vec3{});

        //  Get scene data in correct format.
        //  This command will be run on the graphics thread, so it
        //  must be thread safe.
        view_.high_priority_command([
            t = std::move(triangles),
            v = std::move(vertices),
            vs = model_.scene.get_view_state(),
            pm = model_.scene.get_projection_matrix()
        ](auto& r) {
            r.set_scene(t.data(), t.size(), v.data(), v.size());
            r.set_view_state(vs);
            r.set_projection_matrix(pm);
            r.set_emphasis_colour(
                    {AngularLookAndFeel::emphasis.getFloatRed(),
                     AngularLookAndFeel::emphasis.getFloatGreen(),
                     AngularLookAndFeel::emphasis.getFloatBlue()});
            //  TODO we might need to set other state here too.
        });
    }

    void context_closing(generic_renderer<scene::view>&) override {}

private:
    //  Keep a reference to the global model.
    main_model& model_;

    //  We don't directly use the view, because it needs to run on its own gl
    //  thread.
    //  Instead, we wrap it in an object which supplies async command queues.
    generic_renderer<scene::view> view_;
    model::Connector<generic_renderer<scene::view>> view_connector_{&view_, this};

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
    wayverb::combined::model::sources::scoped_connection sources_connection_;
    wayverb::combined::model::receivers::scoped_connection
            receivers_connection_;

    main_model::waveguide_node_positions_changed::scoped_connection positions_changed_;
    main_model::waveguide_node_pressures_changed::scoped_connection pressures_changed_;
    main_model::raytracer_reflections_generated::scoped_connection reflections_generated_;
    main_model::begun::scoped_connection begun_;
    main_model::finished::scoped_connection finished_;
};

master::master(main_model& model)
        : pimpl_{std::make_unique<impl>(model)} {


    addAndMakeVisible(*pimpl_);
}

master::~master() noexcept = default;

void master::resized() { pimpl_->setBounds(getLocalBounds()); }

}  // namespace scene
