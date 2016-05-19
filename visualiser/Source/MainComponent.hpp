#pragma once

#include "LeftPanel.hpp"
#include "ModelWrapper.hpp"
#include "RightPanel.hpp"

#include "combined/engine.h"

class MainContentComponent final : public Component, public ChangeListener {
public:
    using Engine = WayverbEngine<BufferType::cl>;

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
    model::ChangeConnector render_state_connector{
        &model.get_wrapper().render_state.state, this};

    std::atomic_bool keep_going{true};
    std::thread engine_thread;

    StretchableLayoutManager layout_manager;

    LeftPanel left_panel;
    StretchableLayoutResizerBar resizer_bar;
    RightPanel right_panel;
};
