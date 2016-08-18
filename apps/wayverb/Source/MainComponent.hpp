#pragma once

#include "EngineThread.hpp"
#include "HelpWindow.hpp"
#include "LeftPanel.hpp"
#include "ModelRendererComponent.hpp"

class MainContentComponent final : public Component,
                                   public model::BroadcastListener,
                                   public SettableHelpPanelClient,
                                   public AsyncEngine::Listener {
public:
    MainContentComponent(const copyable_scene_data& scene_data,
                         model::ValueWrapper<model::FullModel>& wrapper);

    void paint(Graphics& g) override;
    void resized() override;

    void receive_broadcast(model::Broadcaster* b) override;

    void engine_encountered_error(AsyncEngine*,
                                  const std::string& str) override;
    void engine_state_changed(AsyncEngine*,
                              wayverb::state state,
                              double progress) override;
    void engine_nodes_changed(
            AsyncEngine*, const aligned::vector<glm::vec3>& positions) override;
    void engine_waveguide_visuals_changed(
            AsyncEngine*,
            const aligned::vector<float>& pressures,
            double current_time) override;
    void engine_raytracer_visuals_changed(
            AsyncEngine*,
            const aligned::vector<aligned::vector<impulse>>& impulses,
            const glm::vec3& sources,
            const glm::vec3& receivers) override;
    void engine_finished(AsyncEngine*) override;

private:
    copyable_scene_data scene_data;
    model::ValueWrapper<model::FullModel>& wrapper;

    model::BroadcastConnector is_rendering_connector{
            &wrapper.render_state.is_rendering, this};

    StretchableLayoutManager layout_manager;

    LeftPanel left_panel;
    StretchableLayoutResizerBar resizer_bar;
    ModelRendererComponent right_panel;

    AsyncEngine engine;
    model::Connector<AsyncEngine> engine_connector{&engine, this};
};
