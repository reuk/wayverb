#include "master_scene_component.h"

#include "Application.h"
#include "CommandIDs.h"

#include "mvc.h"

#include "../UtilityComponents/generic_renderer.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

class master_scene_component::impl final : public Component {
public:
    impl(wayverb::combined::model::app& app)
            : app_{app}
            , controller_{app_} {
        //  TODO Hook up all the actions so that the view is updated when the
        //  model changes.
        
        //  TODO connect to persistent sources/receivers

        //  TODO When the context opens, display the currently loaded scene.
        view_.connect_context_created([&](auto&) {

        });

        //  Make the view visible.
        addAndMakeVisible(view_);
    }

    void resized() override { view_.setBounds(getLocalBounds()); }

private:
    //  We don't directly use the view, because it needs to run on its own gl
    //  thread.
    //  Instead, we wrap it in an object which supplies async command queues.
    generic_renderer<view::scene> view_;

    //  Keep a reference to the global model.
    wayverb::combined::model::app& app_;

    //  This object decides how to interpret user input, and updates the models
    //  as appropriate.
    controller::scene controller_;
};

master_scene_component::master_scene_component(
        wayverb::combined::model::app& app)
        : pimpl_{std::make_unique<impl>(app)} {
    set_help("model viewport",
             "This area displays the currently loaded 3D model. Click and drag "
             "to rotate the model, or use the mouse wheel to zoom in and out.");

    addAndMakeVisible(*pimpl_);
}

master_scene_component::~master_scene_component() noexcept = default;

void master_scene_component::resized() { pimpl_->setBounds(getLocalBounds()); }
