#include "MainComponent.hpp"

#include "combined/config_serialize.h"
#include "common/surface_serialize.h"

#include <iomanip>

MainContentComponent::MainContentComponent(
        const CopyableSceneData& scene_data,
        model::ValueWrapper<model::FullModel>& wrapper)
        : scene_data(scene_data)
        , wrapper(wrapper)
        , left_panel(wrapper, scene_data.get_aabb())
        , resizer_bar(&layout_manager, 1, true)
        , right_panel(scene_data,
                      wrapper.shown_surface,
                      wrapper.persistent.app,
                      wrapper.render_state) {
    set_help("wayverb", "This is the main wayverb app window.");
    auto left_panel_width = 300;
    layout_manager.setItemLayout(
            0, left_panel_width, left_panel_width, left_panel_width);
    auto bar_width = 0;
    layout_manager.setItemLayout(1, bar_width, bar_width, bar_width);
    layout_manager.setItemLayout(2, 300, 10000, 400);

    addAndMakeVisible(left_panel);
    addAndMakeVisible(resizer_bar);
    addAndMakeVisible(right_panel);
}

void MainContentComponent::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
}

void MainContentComponent::resized() {
    std::vector<Component*> components{&left_panel, &resizer_bar, &right_panel};
    layout_manager.layOutComponents(components.data(),
                                    components.size(),
                                    0,
                                    0,
                                    getWidth(),
                                    getHeight(),
                                    false,
                                    true);
}

void MainContentComponent::engine_state_changed(AsyncEngine* u,
                                                engine::State state,
                                                double progress) {
    if (u == &engine) {
        wrapper.render_state.state.set(state);
        wrapper.render_state.progress.set(progress);
    }
}

void MainContentComponent::engine_nodes_changed(
        AsyncEngine* u, const std::vector<cl_float3>& positions) {
    if (u == &engine) {
        right_panel.get_renderer().set_positions(positions);
    }
}

void MainContentComponent::engine_visuals_changed(
        AsyncEngine* u, const std::vector<float>& pressures) {
    if (u == &engine) {
        right_panel.get_renderer().set_pressures(pressures);
    }
}

void MainContentComponent::engine_finished(AsyncEngine* u) {
    if (u == &engine) {
        wrapper.render_state.stop();
    }
}

void MainContentComponent::receive_broadcast(model::Broadcaster* cb) {
    if (cb == &wrapper.render_state.is_rendering) {
        if (wrapper.render_state.is_rendering) {
            FileChooser fc(
                    "save output", File::nonexistent, "*.wav,*.aif,*.aiff");
            if (!fc.browseForFileToSave(true)) {
                wrapper.render_state.stop();
                return;
            }

            auto fpath = fc.getResult().getFullPathName().toStdString();

            auto persistent = wrapper.persistent.get();
            scene_data.set_surfaces(persistent.materials);
            engine.start(
                    persistent, scene_data, wrapper.render_state.visualise);
        } else {
            engine.stop();
        }
    }
}