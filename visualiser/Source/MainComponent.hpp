#pragma once

#include "FullModel.hpp"
#include "LeftPanel.hpp"
#include "ModelRendererComponent.hpp"

#include "combined/engine.h"

class MainContentComponent final : public Component,
                                   public model::BroadcastListener {
public:
    using Engine = engine::WayverbEngine<BufferType::cl>;

    MainContentComponent(const File& root);
    virtual ~MainContentComponent();

    void paint(Graphics& g) override;
    void resized() override;

    void save_as_project();

    void receive_broadcast(model::Broadcaster* cb) override;

private:
    void join_engine_thread();

    SceneData scene_data;

    model::FullModel model;
    model::ValueWrapper<model::FullModel> wrapper{nullptr, model};

    model::BroadcastConnector is_rendering_connector{
        &wrapper.render_state.is_rendering, this};

    std::atomic_bool keep_going{true};
    std::thread engine_thread;

    StretchableLayoutManager layout_manager;

    LeftPanel left_panel;
    StretchableLayoutResizerBar resizer_bar;
    ModelRendererComponent right_panel;
};
