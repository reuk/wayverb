#include "MainComponent.hpp"

#include "combined/config_serialize.h"
#include "common/surface_serialize.h"

#include "LoadFiles.hpp"

#include <iomanip>

MainContentComponent::MainContentComponent(const File& root)
        : scene_data(load_model(root, load_materials(root)))
        , model(
              nullptr,
              model::FullModel{
                  load_config(root),
                  scene_data.get_materials(),
                  {
                      SceneData::Material{
                          "concrete_floor",
                          Surface{
                              {0.99, 0.97, 0.95, 0.98, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "brickwork",
                          Surface{
                              {0.99, 0.98, 0.98, 0.97, 0.97, 0.96, 0.96, 0.96},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "rough_lime_wash",
                          Surface{
                              {0.98, 0.97, 0.96, 0.95, 0.96, 0.97, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "limestone",
                          Surface{
                              {0.98, 0.98, 0.97, 0.96, 0.95, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "wooden_door",
                          Surface{
                              {0.86, 0.9, 0.94, 0.92, 0.9, 0.9, 0.9, 0.9},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "rough_concrete",
                          Surface{
                              {0.98, 0.97, 0.97, 0.97, 0.96, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "smooth_floor",
                          Surface{
                              {0.98, 0.97, 0.97, 0.97, 0.97, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "lead_glazing",
                          Surface{
                              {0.7, 0.8, 0.86, 0.9, 0.95, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "leaded_glazing",
                          Surface{
                              {0.85, 0.7, 0.82, 0.9, 0.95, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "chairs__2",
                          Surface{
                              {0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "takapaaty_flat__2",
                          Surface{
                              {0.85, 0.9, 0.94, 0.96, 0.96, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "double_glazing",
                          Surface{
                              {0.85, 0.95, 0.97, 0.97, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "audience_floor",
                          Surface{
                              {0.91, 0.94, 0.95, 0.95, 0.95, 0.96, 0.96, 0.96},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "katto_flat__2",
                          Surface{
                              {0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "verb_chamber",
                          Surface{
                              {0.99, 0.99, 0.99, 0.98, 0.98, 0.96, 0.96, 0.96},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "plywood_panels",
                          Surface{
                              {0.58, 0.79, 0.9, 0.92, 0.94, 0.94, 0.94, 0.94},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "smooth_concrete",
                          Surface{{0.6, 0.8, 0.9, 0.95, 0.97, 0.97, 0.96, 0.95},
                                  {0.99,
                                   0.99,
                                   0.98,
                                   0.98,
                                   0.98,
                                   0.95,
                                   0.95,
                                   0.95}}},
                      SceneData::Material{
                          "glass_window",
                          Surface{
                              {0.9, 0.95, 0.96, 0.97, 0.97, 0.97, 0.97, 0.97},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "ceramic_tiles",
                          Surface{
                              {0.99, 0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "pile_carpet",
                          Surface{
                              {0.97, 0.91, 0.75, 0.69, 0.6, 0.56, 0.56, 0.56},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "4cm_planks",
                          Surface{
                              {0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "point_brickwork",
                          Surface{
                              {0.92, 0.91, 0.88, 0.84, 0.78, 0.76, 0.76, 0.76},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "marble_floor",
                          Surface{
                              {0.99, 0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "wooden_lining",
                          Surface{
                              {0.73, 0.77, 0.78, 0.85, 0.9, 0.93, 0.94, 0.94},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "hull__2",
                          Surface{
                              {0.99, 0.99, 0.99, 0.98, 0.98, 0.96, 0.96, 0.96},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "stucco_brick",
                          Surface{
                              {0.97, 0.97, 0.97, 0.96, 0.95, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "vasen_flat__2",
                          Surface{
                              {0.73, 0.77, 0.78, 0.85, 0.9, 0.93, 0.94, 0.94},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "double_glazing_2",
                          Surface{
                              {0.9, 0.93, 0.95, 0.97, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "hard_wall",
                          Surface{
                              {0.98, 0.98, 0.97, 0.97, 0.96, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "thin_carpet",
                          Surface{
                              {0.98, 0.96, 0.92, 0.8, 0.75, 0.6, 0.6, 0.6},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "etupaaty_flat__2",
                          Surface{
                              {0.92, 0.91, 0.88, 0.84, 0.78, 0.76, 0.76, 0.76},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "oikea_flat__2",
                          Surface{
                              {0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "smooth_brickwork",
                          Surface{
                              {0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "wood_40mm_studs",
                          Surface{
                              {0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "cotton_carpet",
                          Surface{
                              {0.93, 0.69, 0.51, 0.19, 0.34, 0.46, 0.52, 0.52},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "3mm_glass",
                          Surface{
                              {0.92, 0.96, 0.97, 0.97, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "tufted_carpet",
                          Surface{
                              {0.9, 0.6, 0.37, 0.3, 0.37, 0.12, 0.12, 0.12},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "plasterboard",
                          Surface{
                              {0.85, 0.9, 0.94, 0.96, 0.96, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                      SceneData::Material{
                          "stage_floor",
                          Surface{
                              {0.9, 0.93, 0.94, 0.94, 0.94, 0.94, 0.94, 0.94},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  },
                  model::FullReceiverConfig{},
                  model::RenderState{}})
        , left_panel(model.get_wrapper())
        , resizer_bar(&layout_manager, 1, true)
        , right_panel(scene_data,
                      model.get_wrapper().combined,
                      model.get_wrapper().render_state) {
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

void MainContentComponent::save_as_project() {
}

void MainContentComponent::join_engine_thread() {
    if (engine_thread.joinable()) {
        engine_thread.join();
    }
}

static auto to_string(MainContentComponent::Engine::State state) {
    switch (state) {
        case MainContentComponent::Engine::State::starting_raytracer:
            return "starting raytracer";
        case MainContentComponent::Engine::State::running_raytracer:
            return "running raytracer";
        case MainContentComponent::Engine::State::finishing_raytracer:
            return "finishing raytracer";
        case MainContentComponent::Engine::State::starting_waveguide:
            return "starting waveguide";
        case MainContentComponent::Engine::State::running_waveguide:
            return "running waveguide";
        case MainContentComponent::Engine::State::finishing_waveguide:
            return "finishing waveguide";
        case MainContentComponent::Engine::State::postprocessing:
            return "postprocessing";
    }
}

void MainContentComponent::changeListenerCallback(ChangeBroadcaster* cb) {
    if (cb == &model.get_wrapper().render_state.state) {
        switch (model.get_wrapper().render_state.state) {
            case model::RenderState::State::started:
                keep_going = true;

                std::cout << "start render on dedicated thread here"
                          << std::endl;

                engine_thread = std::thread([this] {
                    ComputeContext compute_context;
                    try {
                        Engine engine(compute_context,
                                      scene_data,
                                      model.get_wrapper().combined.source,
                                      model.get_wrapper().combined.mic,
                                      model.get_wrapper()
                                          .combined.get()
                                          .get_waveguide_sample_rate(),
                                      model.get_wrapper().combined.rays,
                                      model.get_wrapper().combined.impulses,
                                      model.get_wrapper().combined.sample_rate);

                        struct Callback {
                            void operator()(Engine::State state,
                                            double progress) const {
                                std::cout << std::setw(30) << to_string(state)
                                          << std::setw(10) << progress
                                          << std::endl;
                            }
                        };

                        Callback callback;

                        engine.run(keep_going, callback);

                        //  TODO process or whatever

                        //  TODO write out

                        //  notify
                        model.get_wrapper().render_state.state.set(
                            model::RenderState::State::stopped);
                    } catch (const std::runtime_error& e) {
                        std::cout << "wayverb thread error: " << e.what()
                                  << std::endl;
                    }
                });

                break;
            case model::RenderState::State::stopped:
                keep_going = false;
                join_engine_thread();
                break;
        }
    }
}