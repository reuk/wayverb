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

    setSize(800, 500);
}

MainContentComponent::~MainContentComponent() {
    quit_render_thread();
}

void MainContentComponent::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
}

void MainContentComponent::resized() {
    Component* component[]{&left_panel, &resizer_bar, &right_panel};
    layout_manager.layOutComponents(
            component, 3, 0, 0, getWidth(), getHeight(), false, true);
}

void MainContentComponent::handleAsyncUpdate() {
    assert(engine_thread.joinable());
    quit_render_thread();
    wrapper.render_state.stop();
}

void MainContentComponent::quit_render_thread() {
    keep_going = false;
    if (engine_thread.joinable()) {
        engine_thread.join();
    }
}

float max_reflectivity(const VolumeType& vt) {
    return *proc::max_element(vt.s);
}

float max_reflectivity(const Surface& surface) {
    return std::max(max_reflectivity(surface.diffuse),
                    max_reflectivity(surface.specular));
}

float max_reflectivity(const SceneData::Material& material) {
    return max_reflectivity(material.surface);
}

float max_reflectivity(const std::vector<SceneData::Material>& materials) {
    return std::accumulate(materials.begin() + 1,
                           materials.end(),
                           max_reflectivity(materials.front()),
                           [](const auto& i, const auto& j) {
                               return std::max(i, max_reflectivity(j));
                           });
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

            keep_going = true;
            engine_thread = std::thread([this, fpath] {
                ComputeContext compute_context;
                try {
                    auto callback = [this](auto state, auto progress) {
                        wrapper.render_state.state.set(state);
                        wrapper.render_state.progress.set(progress);
                    };

                    //  refresh the scene with current materials
                    scene_data.set_surfaces(wrapper.persistent.materials);

                    //  compute ideal number of impulses
                    auto impulses = compute_optimum_reflection_number(
                            Decibels::decibelsToGain(-48.0),
                            max_reflectivity(wrapper.persistent.materials));
                    std::cerr << "impulses estimated: " << impulses
                              << std::endl;

                    //  init the engine
                    callback(engine::State::initialising, 1.0);
                    Engine engine(compute_context,
                                  scene_data,
                                  wrapper.persistent.app.source,
                                  wrapper.persistent.app.receiver,
                                  wrapper.persistent.app.get()
                                          .get_waveguide_sample_rate(),
                                  wrapper.persistent.app.rays,
                                  impulses,
                                  //    TODO get samplerate from dialog?
                                  44100);

                    //  check that source and mic are inside model
                    auto check_position = [](auto valid,
                                             const std::string& str) {
                        if (!valid) {
                            NativeMessageBox::showMessageBoxAsync(
                                    AlertWindow::AlertIconType::WarningIcon,
                                    str + " position is invalid",
                                    "It looks like that " + str +
                                            " position is outside the "
                                            "waveguide mesh. "
                                            "Make sure the 3D model is "
                                            "completely "
                                            "closed, and the " +
                                            str + " is inside.");
                            throw std::runtime_error(str + " is outside mesh");
                        }
                    };

                    check_position(engine.get_mic_position_is_valid(),
                                   "microphone");
                    check_position(engine.get_source_position_is_valid(),
                                   "source");

                    //  now run the simulation proper
                    auto run = [this, &engine, &callback] {
                        return engine.run(keep_going, callback);
                    };

                    auto run_visualised = [this, &engine, &callback] {
                        right_panel.get_renderer().set_positions(
                                engine.get_node_positions());
                        return engine.run_visualised(
                                keep_going, callback, [this](const auto& i) {
                                    right_panel.get_renderer().set_pressures(i);
                                });
                    };
                    auto intermediate = wrapper.render_state.visualise
                                                ? run_visualised()
                                                : run();
                    engine.attenuate(intermediate, callback);
                    //  TODO write out

                    //  Launch viewer window or whatever

                    //  if anything goes wrong, flag it up on stdout and quit
                    //  the thread
                } catch (const std::runtime_error& e) {
                    std::cout << "wayverb thread error: " << e.what()
                              << std::endl;
                } catch (...) {
                    std::cout << "something went pretty wrong so the render "
                                 "thread is qutting now"
                              << std::endl;
                }
                //  notify
                keep_going = false;
                this->triggerAsyncUpdate();
            });
        } else {
            //  user cancelled the render
            keep_going = false;
        }
    }
}