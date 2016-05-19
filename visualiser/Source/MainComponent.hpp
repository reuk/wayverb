#pragma once

#include "LeftPanel.hpp"
#include "ModelRendererComponent.hpp"
#include "ModelWrapper.hpp"

#include "combined/engine.h"

class MainContentComponent final : public Component, public ChangeListener {
public:
    using Engine = engine::WayverbEngine<BufferType::cl>;

    MainContentComponent(const File& root);
    virtual ~MainContentComponent();

    void paint(Graphics& g) override;
    void resized() override;

    void save_as_project();

    void changeListenerCallback(ChangeBroadcaster* cb) override;

private:
    void join_engine_thread();

    SceneData scene_data;
    model::ValueWithWrapper<model::FullModel> model;
    model::ChangeConnector is_rendering_connector{
        &model.get_wrapper().render_state.is_rendering, this};

    std::atomic_bool keep_going{true};
    std::thread engine_thread;

    StretchableLayoutManager layout_manager;

    LeftPanel left_panel;
    StretchableLayoutResizerBar resizer_bar;
    ModelRendererComponent right_panel;
};
