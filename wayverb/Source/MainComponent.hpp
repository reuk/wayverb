#pragma once

#include "HelpWindow.hpp"
#include "LeftPanel.hpp"
#include "ModelRendererComponent.hpp"

#include "combined/engine.h"

class MainContentComponent final : public Component,
                                   public model::BroadcastListener,
                                   public SettableHelpPanelClient,
                                   public AsyncUpdater {
public:
    using Engine = engine::WayverbEngine<BufferType::cl>;

    MainContentComponent(const CopyableSceneData& scene_data,
                         model::ValueWrapper<model::FullModel>& wrapper);

    virtual ~MainContentComponent();

    void paint(Graphics& g) override;
    void resized() override;

    void receive_broadcast(model::Broadcaster* b) override;

    void quit_render_thread();

private:
    //  will be called to quit the render thread
    void handleAsyncUpdate() override;

    CopyableSceneData scene_data;
    model::ValueWrapper<model::FullModel>& wrapper;

    model::BroadcastConnector is_rendering_connector{
            &wrapper.render_state.is_rendering, this};

    StretchableLayoutManager layout_manager;

    LeftPanel left_panel;
    StretchableLayoutResizerBar resizer_bar;
    ModelRendererComponent right_panel;

    std::atomic_bool keep_going{true};
    std::thread engine_thread;
};
