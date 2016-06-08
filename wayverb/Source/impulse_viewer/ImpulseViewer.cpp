#include "ImpulseViewer.hpp"

namespace {
auto load_file(const File& file) {
    AudioFormatManager m;
    m.registerBasicFormats();
    std::unique_ptr<AudioFormatReader> reader(m.createReaderFor(file));

    AudioSampleBuffer buffer(reader->numChannels, reader->lengthInSamples);
    reader->read(&buffer, 0, reader->lengthInSamples, 0, true, true);

    std::vector<std::vector<float>> ret(
        reader->numChannels, std::vector<float>(reader->lengthInSamples));
    for (auto i = 0; i != reader->numChannels; ++i) {
        std::copy(buffer.getReadPointer(i),
                  buffer.getReadPointer(i) + reader->lengthInSamples,
                  ret[i].begin());
    }
    return ret;
}
}  // namespace

//  launch thread
//  read file in small chunks until end, then join thread

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
                renderer.get_renderer().set_mode(ImpulseRenderer::Mode::waveform);
                break;
            case 1:
                renderer.get_renderer().set_mode(ImpulseRenderer::Mode::waterfall);
                break;
        }
    }
}