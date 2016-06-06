#include "ImpulseViewer.hpp"

ImpulseViewer::ImpulseViewer(): tabs(TabbedButtonBar::Orientation::TabsAtTop) {
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