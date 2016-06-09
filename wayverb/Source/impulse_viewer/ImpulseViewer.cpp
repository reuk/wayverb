#include "ImpulseViewer.hpp"

ImpulseViewer::DefaultAudioFormatManager::DefaultAudioFormatManager() {
    registerBasicFormats();
}

ImpulseViewer::DefaultAudioDeviceManager::DefaultAudioDeviceManager() {
    initialiseWithDefaultDevices(0, 2);
}

ImpulseViewer::ImpulseViewer(const File& file)
        : audio_format_reader_source(audio_format_manager.createReaderFor(file),
                                     true)
        , renderer(audio_transport_source)
        , tabs(TabbedButtonBar::Orientation::TabsAtTop)
        , transport(audio_transport_source) {
    audio_transport_source.setSource(&audio_format_reader_source);
    audio_source_player.setSource(&audio_transport_source);
    audio_device_manager.addAudioCallback(&audio_source_player);

    tabs.addTab("waveform", Colours::darkgrey, -1);
    tabs.addTab("waterfall", Colours::darkgrey, -1);

    addAndMakeVisible(renderer);
    addAndMakeVisible(tabs);
    addAndMakeVisible(transport);

    setSize(800, 500);

    load_from(file);
}

ImpulseViewer::~ImpulseViewer() noexcept {
    audio_device_manager.removeAudioCallback(&audio_source_player);
}

void ImpulseViewer::resized() {
    auto bounds = getLocalBounds();
    auto top = bounds.removeFromTop(40);
    renderer.setBounds(bounds);

    transport.setBounds(top.removeFromLeft(200));
    tabs.setBounds(top);
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

void ImpulseViewer::load_from(const File& f) {
    renderer.get_renderer().load_from(audio_format_manager, f);
}