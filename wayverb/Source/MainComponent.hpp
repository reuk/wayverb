#pragma once

#include "EngineThread.hpp"
#include "HelpWindow.hpp"
#include "LeftPanel.hpp"
#include "ModelRendererComponent.hpp"

class MainContentComponent final : public Component, public SettableHelpPanelClient {
public:
    MainContentComponent();

    void paint(Graphics& g) override;
    void resized() override;

/*
    void engine_encountered_error(AsyncEngine*,
                                  const std::string& str) override;
    void engine_state_changed(AsyncEngine*,
                              wayverb::combined::state state,
                              double progress) override;
    void engine_nodes_changed(
            AsyncEngine*, const util::aligned::vector<glm::vec3>& positions) override;
    void engine_waveguide_visuals_changed(
            AsyncEngine*,
            const util::aligned::vector<float>& pressures,
            double current_time) override;
    void engine_raytracer_visuals_changed(
            AsyncEngine*,
            const util::aligned::vector<util::aligned::vector<wayverb::raytracer::reflection>>& reflections,
            const glm::vec3& sources,
            const glm::vec3& receivers) override;
    void engine_finished(AsyncEngine*) override;
    */

private:
    /*
    model::ValueWrapper<model::FullModel>& wrapper;

    model::BroadcastConnector is_rendering_connector{
            &wrapper.render_state.is_rendering, this};
    */

    StretchableLayoutManager layout_manager;

    LeftPanel left_panel;
    StretchableLayoutResizerBar resizer_bar;
    ModelRendererComponent right_panel;

    //model::Connector<LeftPanel> panel_connector{&left_panel, &right_panel};

    AsyncEngine engine;
    //model::Connector<AsyncEngine> engine_connector{&engine, this};
};
