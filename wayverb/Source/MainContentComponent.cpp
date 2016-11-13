#include "MainContentComponent.h"

#include "core/serialize/surface.h"

#include <iomanip>
#include <type_traits>

MainContentComponent::MainContentComponent(wayverb::combined::model::app& app)
        : left_panel_{app}
        , resizer_bar_{&layout_manager_, 1, true}
        , right_panel_{app} {
    set_help("wayverb", "This is the main wayverb app window.");
    const auto left_panel_width = 300;
    layout_manager_.setItemLayout(
            0, left_panel_width, left_panel_width, left_panel_width);
    const auto bar_width = 0;
    layout_manager_.setItemLayout(1, bar_width, bar_width, bar_width);
    layout_manager_.setItemLayout(2, 300, 10000, 400);

    addAndMakeVisible(left_panel_);
    addAndMakeVisible(resizer_bar_);
    addAndMakeVisible(right_panel_);
}

void MainContentComponent::paint(Graphics& g) { g.fillAll(Colours::darkgrey); }

void MainContentComponent::resized() {
    Component* components[] = {&left_panel_, &resizer_bar_, &right_panel_};
    layout_manager_.layOutComponents(components,
                                     std::extent<decltype(components), 0>{},
                                     0,
                                     0,
                                     getWidth(),
                                     getHeight(),
                                     false,
                                     true);
}
