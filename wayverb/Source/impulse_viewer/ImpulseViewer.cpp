#include "ImpulseViewer.hpp"

ImpulseViewer::ImpulseViewer(const File& file)
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
                renderer.get_renderer().set_mode(
                        ImpulseRenderer::Mode::waveform);
                break;
            case 1:
                renderer.get_renderer().set_mode(
                        ImpulseRenderer::Mode::waterfall);
                break;
        }
    }
}

void ImpulseViewer::load_from(AudioFormatManager& afm, const File& f) {
    renderer.get_renderer().load_from(afm, f);
}