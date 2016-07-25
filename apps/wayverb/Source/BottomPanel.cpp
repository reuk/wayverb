#include "BottomPanel.hpp"

BottomPanel::BottomPanel(model::ValueWrapper<model::RenderState>& render_state)
        : render_state(render_state)
        , bar(progress) {
    for (auto i :
         {&is_rendering_connector, &state_connector, &progress_connector}) {
        i->trigger();
    }

    addAndMakeVisible(bar);
    addAndMakeVisible(button);
}

void BottomPanel::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
}

void BottomPanel::resized() {
    auto button_width = 100;
    auto bounds = getLocalBounds().reduced(2, 2);

    bar.setBounds(bounds.withTrimmedRight(button_width + 1));
    button.setBounds(bounds.removeFromRight(button_width - 1));
}

void BottomPanel::buttonClicked(Button* b) {
    if (b == &button) {
        if (render_state.is_rendering.get()) {
            render_state.stop();
        } else {
            render_state.start();
        }
    }
}

void BottomPanel::receive_broadcast(model::Broadcaster* cb) {
    if (cb == &render_state.is_rendering) {
        button.setButtonText(render_state.is_rendering.get() ? "cancel"
                                                             : "render");
    } else if (cb == &render_state.state) {
        bar.setTextToDisplay(wayverb::to_string(render_state.state.get()));
    } else if (cb == &render_state.progress) {
        progress = render_state.progress.get();
    }
}