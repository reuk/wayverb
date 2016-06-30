#include "ModelRendererComponent.hpp"

#include "OtherComponents/MoreConversions.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "combined/model.h"

#include "common/boundaries.h"

ModelRendererComponent::ModelRendererComponent(
        const CopyableSceneData &model,
        model::ValueWrapper<int> &shown_surface,
        model::ValueWrapper<model::App> &app,
        model::ValueWrapper<model::RenderState> &render_state)
        : BaseRendererComponent(model)
        , model(model)
        , shown_surface(shown_surface)
        , app(app)
        , render_state(render_state) {
    set_help("model viewport",
             "This area displays the currently loaded 3D model. Click and drag "
             "to rotate the model, or use the mouse wheel to zoom in and out.");
}

namespace {
auto get_receiver_directions(const model::ValueWrapper<model::App> &app) {
}
}  // namespace

void ModelRendererComponent::receive_broadcast(model::Broadcaster *cb) {
    if (cb == &shown_surface) {
        renderer.set_highlighted(shown_surface);
    } else if (cb == &app.source) {
        renderer.set_sources(app.source);
    } else if (cb == &app.receiver_settings) {
        renderer.set_receivers(app.receiver_settings);
    } else if (cb == &render_state.is_rendering) {
        renderer.set_rendering(render_state.is_rendering);
    }
}

void ModelRendererComponent::changeListenerCallback(ChangeBroadcaster *u) {
    if (u == &renderer) {
        for (auto i : {&shown_connector,
                       &receiver_settings_connector,
                       &source_connector,
                       &is_rendering_connector,
                       &facing_direction_connector}) {
            i->trigger();
        }
    }
}

void ModelRendererComponent::source_dragged(SceneRenderer *,
                                            const std::vector<glm::vec3> &v) {
    assert(app.source.get().size() == v.size());
    for (auto i = 0u; i != v.size(); ++i) {
        app.source[i].set(glm::clamp(
                v[i], model.get_aabb().get_c0(), model.get_aabb().get_c1()));
    }
}
void ModelRendererComponent::receiver_dragged(SceneRenderer *,
                                              const std::vector<glm::vec3> &v) {
    assert(app.receiver_settings.get().size() == v.size());
    for (auto i = 0u; i != v.size(); ++i) {
        app.receiver_settings[i].position.set(glm::clamp(
                v[i], model.get_aabb().get_c0(), model.get_aabb().get_c1()));
    }
}