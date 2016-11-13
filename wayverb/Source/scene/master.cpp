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

        //  When view is initialised, set the scene correctly
        //  The callback will be run on the main message thread, so we can do
        //  what we like (within reason).
        view_.connect_context_created([&](auto& renderer) {
            //  Get scene data in correct format.
            const auto scene_data = app_.project.get_scene_data();
            auto triangles = scene_data.get_triangles();
            auto vertices =
                    util::map_to_vector(begin(scene_data.get_vertices()),
                                        end(scene_data.get_vertices()),
                                        wayverb::core::to_vec3{});


            //  This command will be run on the graphics thread, so it must
            //  be thread safe.
            renderer.command(
                    [ t = std::move(triangles),
                      v = std::move(vertices) ](auto& r) {
                        r.set_scene(t.data(), t.size(), v.data(), v.size());
                    });
        });

        //  Make the view visible.
        addAndMakeVisible(view_);
    }

    void resized() override { view_.setBounds(getLocalBounds()); }

    void mouse_down(const MouseEvent& e) { controller_.mouse_down(e); }

    void mouse_drag(const MouseEvent& e) { controller_.mouse_drag(e); }

    void mouse_up(const MouseEvent& e) { controller_.mouse_up(e); }

    void mouse_wheel_move(const MouseEvent& e, const MouseWheelDetails& d) {
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

void master::mouseDown(const MouseEvent& e) { pimpl_->mouse_down(e); }
void master::mouseDrag(const MouseEvent& e) { pimpl_->mouse_drag(e); }
void master::mouseUp(const MouseEvent& e) { pimpl_->mouse_up(e); }
void master::mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& d) {
    pimpl_->mouse_wheel_move(e, d);
}

}  // namespace scene
