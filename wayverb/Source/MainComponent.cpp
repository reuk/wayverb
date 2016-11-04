#include "MainComponent.hpp"

#include "core/serialize/surface.h"

#include <iomanip>

MainContentComponent::MainContentComponent()
        : left_panel()
        , resizer_bar(&layout_manager, 1, true) {
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

void MainContentComponent::paint(Graphics& g) { g.fillAll(Colours::darkgrey); }

void MainContentComponent::resized() {
    util::aligned::vector<Component*> components{
            &left_panel, &resizer_bar, &right_panel};
    layout_manager.layOutComponents(components.data(),
                                    components.size(),
                                    0,
                                    0,
                                    getWidth(),
                                    getHeight(),
                                    false,
                                    true);
}

/*
void MainContentComponent::engine_encountered_error(AsyncEngine* u,
                                                    const std::string& str) {
    if (u == &engine) {
        //  TODO
        //  report the error somehow
    }
}

void MainContentComponent::engine_state_changed(AsyncEngine* u,
                                                wayverb::combined::state state,
                                                double progress) {

}

void MainContentComponent::engine_nodes_changed(
        AsyncEngine* u, const util::aligned::vector<glm::vec3>& positions) {

}

void MainContentComponent::engine_waveguide_visuals_changed(
        AsyncEngine* u,
        const util::aligned::vector<float>& pressures,
        double current_time) {

}

void MainContentComponent::engine_raytracer_visuals_changed(
        AsyncEngine* u,
        const util::aligned::vector<util::aligned::vector<wayverb::raytracer::reflection>>& reflections,
        const glm::vec3& source,
        const glm::vec3& receiver) {

}

void MainContentComponent::engine_finished(AsyncEngine* u) {

}
*/
