#include "ImpulseViewer.hpp"

ImpulseViewer::ImpulseViewer()
        : tabs(TabbedButtonBar::Orientation::TabsAtTop) {
    tabs.addTab("waveform", Colours::darkgrey, -1);
    tabs.addTab("waterfall", Colours::darkgrey, -1);

    addAndMakeVisible(renderer);
    addAndMakeVisible(tabs);

    setSize(800, 500);
}

void ImpulseViewer::resized() {
    auto bounds = getLocalBounds();
    tabs.setBounds(bounds.removeFromTop(30));
    renderer.setBounds(bounds);
}

void ImpulseViewer::changeListenerCallback(ChangeBroadcaster* cb) {
    if (cb == &tabs) {
        switch (tabs.getCurrentTabIndex()) {
            case 0:
                renderer.set_mode(ImpulseRenderer::Mode::waveform);
                break;
            case 1:
                renderer.set_mode(ImpulseRenderer::Mode::waterfall);
                break;
        }
    }
}