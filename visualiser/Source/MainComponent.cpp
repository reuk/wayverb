#include "MainComponent.hpp"

#include "combined/config_serialize.h"
#include "common/surface_serialize.h"

#include <iomanip>

MainContentComponent::MainContentComponent(
    SceneData& scene_data, model::ValueWrapper<model::FullModel>& wrapper)
        : scene_data(scene_data)
        , wrapper(wrapper)
        , left_panel(wrapper, scene_data)
        , resizer_bar(&layout_manager, 1, true)
        , right_panel(scene_data,
                      wrapper.shown_surface,
                      wrapper.combined,
                      wrapper.render_state) {
    auto left_panel_width = 300;
    layout_manager.setItemLayout(
        0, left_panel_width, left_panel_width, left_panel_width);
    auto bar_width = 0;
    layout_manager.setItemLayout(1, bar_width, bar_width, bar_width);
    layout_manager.setItemLayout(2, 300, 10000, 400);

    addAndMakeVisible(left_panel);
    addAndMakeVisible(resizer_bar);
    addAndMakeVisible(right_panel);

    setSize(800, 500);
}

MainContentComponent::~MainContentComponent() {
    join_engine_thread();
}

void MainContentComponent::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
}

void MainContentComponent::resized() {
    Component* component[]{&left_panel, &resizer_bar, &right_panel};
    layout_manager.layOutComponents(
        component, 3, 0, 0, getWidth(), getHeight(), false, true);
}

void MainContentComponent::join_engine_thread() {
    keep_going = false;
    if (engine_thread.joinable()) {
        engine_thread.join();
    }
}

void MainContentComponent::receive_broadcast(model::Broadcaster* cb) {
    if (cb == &wrapper.render_state.is_rendering) {
        if (wrapper.render_state.is_rendering) {
            keep_going = true;
            engine_thread = std::thread([this] {
                ComputeContext compute_context;
                try {
                    auto callback = [this](auto state, auto progress) {
                        wrapper.render_state.state.set(state);
                        wrapper.render_state.progress.set(progress);
                    };

                    callback(engine::State::initialising, 1.0);
                    Engine engine(
                        compute_context,
                        scene_data,
                        wrapper.combined.source,
                        wrapper.combined.mic,
                        wrapper.combined.get().get_waveguide_sample_rate(),
                        wrapper.combined.rays,
                        wrapper.combined.impulses,
                        wrapper.combined.sample_rate);

                    auto run = [this, &engine, &callback] {
                        return engine.run(keep_going, callback);
                    };

                    auto run_visualised = [this, &engine, &callback] {
                        right_panel.set_positions(engine.get_node_positions());
                        //  TODO this is not great, but I know WHY it's not
                        //  great so it's only 50% unforgivable
                        return engine.run_visualised(
                            keep_going, callback, [this](const auto& i) {
                                right_panel.set_pressures(i);
                            });
                    };
                    auto intermediate = wrapper.render_state.visualise
                                            ? run_visualised()
                                            : run();
                    engine.attenuate(intermediate, callback);
                    //  TODO write out

                } catch (const std::runtime_error& e) {
                    std::cout << "wayverb thread error: " << e.what()
                              << std::endl;
                } catch (...) {
                    std::cout << "something went pretty wrong so the render "
                                 "thread is qutting now"
                              << std::endl;
                }
                //  notify
                wrapper.render_state.stop();
            });
        } else {
            join_engine_thread();
        }
    }
}