#pragma once

#include "FullModel.hpp"
#include "ImpulseRenderer.hpp"
#include "Transport.hpp"
#include "Ruler.hpp"

class ImpulseViewer : public Component,
                      public ChangeListener,
                      public ApplicationCommandTarget,
                      public Ruler::Listener {
public:
    ImpulseViewer(const File& file);
    virtual ~ImpulseViewer() noexcept;

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

    void load_from(const File& f);

    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

    void rulerMouseDown(Ruler* ruler, const MouseEvent& e, float time) override;
    void rulerMouseUp(Ruler* ruler, const MouseEvent& e) override;
    void rulerDragged(Ruler* ruler, const MouseEvent& e) override;
    void rulerDoubleClicked(Ruler* ruler, const MouseEvent& e) override;

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
    TabbedButtonBar tabs;
    Transport transport;

    model::Connector<ChangeBroadcaster> tabs_connector{&tabs, this};
    model::Connector<Ruler> ruler_connector{&ruler, this};
};
