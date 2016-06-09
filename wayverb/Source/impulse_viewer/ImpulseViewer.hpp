#pragma once

#include "FullModel.hpp"
#include "ImpulseRenderer.hpp"
#include "Transport.hpp"

class ImpulseViewer : public Component, public ChangeListener {
public:
    ImpulseViewer(const File& file);
    virtual ~ImpulseViewer() noexcept;

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

    void load_from(const File& f);

private:
    class DefaultAudioFormatManager : public AudioFormatManager {
    public:
        DefaultAudioFormatManager();
    };

    class DefaultAudioDeviceManager : public AudioDeviceManager {
    public:
        DefaultAudioDeviceManager();
    };

    DefaultAudioDeviceManager audio_device_manager;
    DefaultAudioFormatManager audio_format_manager;
    AudioFormatReaderSource audio_format_reader_source;
    AudioTransportSource audio_transport_source;
    AudioSourcePlayer audio_source_player;

    ImpulseRendererComponent renderer;
    TabbedButtonBar tabs;
    Transport transport;

    model::Connector<ChangeBroadcaster> tabs_connector{&tabs, this};
};
