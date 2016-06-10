#pragma once

#include "FullModel.hpp"
#include "ImpulseRenderer.hpp"
#include "Ruler.hpp"
#include "Transport.hpp"

class ImpulseViewer : public Component,
                      public ApplicationCommandTarget,
                      public Button::Listener {
public:
    ImpulseViewer(const File& file);
    virtual ~ImpulseViewer() noexcept;

    void resized() override;

    void load_from(const File& f);

    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

    void buttonClicked(Button* b) override;

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
    Ruler ruler;

    TextButton waterfall_button;
    TextButton waveform_button;

    Transport transport;

    model::Connector<TextButton> waterfall_button_connector{&waterfall_button,
                                                            this};
    model::Connector<TextButton> waveform_button_connector{&waveform_button,
                                                           this};
    model::Connector<Ruler> ruler_connector{&ruler, &renderer.get_renderer()};
};
