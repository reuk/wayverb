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
    using Engine = engine::WayverbEngine<BufferType::cl>;

    MainContentComponent(const CopyableSceneData& scene_data,
                         model::ValueWrapper<model::FullModel>& wrapper);

    void paint(Graphics& g) override;
    void resized() override;

    void receive_broadcast(model::Broadcaster* b) override;

    void engine_state_changed(AsyncEngine*,
                              engine::State state,
                              double progress) override;
    void engine_nodes_changed(AsyncEngine*,
                              const std::vector<cl_float3>& positions) override;
    void engine_visuals_changed(AsyncEngine*,
                                const std::vector<float>& pressures) override;
    void engine_finished(AsyncEngine*) override;

private:
    CopyableSceneData scene_data;
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