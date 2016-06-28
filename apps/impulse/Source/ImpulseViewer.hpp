#pragma once

#include "ImpulseRenderer.hpp"

#include "OtherComponents/lib_model/connector.hpp"

#include "UtilityComponents/DefaultAudio.hpp"
#include "UtilityComponents/Ruler.hpp"
#include "UtilityComponents/Transport.hpp"

class ImpulseViewer : public Component,
                      public ApplicationCommandTarget,
                      public Button::Listener,
                      public PlaybackViewManager::Listener,
                      public ScrollBar::Listener,
                      public ComboBox::Listener {
public:
    ImpulseViewer(AudioDeviceManager& audio_device_manager,
                  AudioFormatManager& audio_format_manager,
                  const File& file);
    virtual ~ImpulseViewer() noexcept;

    void resized() override;

    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

    void buttonClicked(Button* b) override;

    void max_range_changed(PlaybackViewManager* r,
                           const Range<double>& range) override;
    void visible_range_changed(PlaybackViewManager* r,
                               const Range<double>& range) override;
    void current_time_changed(PlaybackViewManager* r, double time) override;

    void scrollBarMoved(ScrollBar* s, double new_range_start) override;

    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

private:
    AudioDeviceManager& audio_device_manager;

    AudioFormatReaderSource audio_format_reader_source;
    AudioTransportSource audio_transport_source;
    TransportViewManager transport_view_manager;
    AudioSourcePlayer audio_source_player;

    ImpulseRendererComponent renderer;
    Ruler ruler;

    ComboBox channel_combo_box;

    TextButton waterfall_button;
    TextButton waveform_button;

    ToggleButton follow_playback_button;

    ScrollBar scroll_bar;

    Transport transport;

    model::Connector<PlaybackViewManager> pvm_connector_0{
            &transport_view_manager, this};
    model::Connector<ComboBox> combo_box_connector{&channel_combo_box, this};
    model::Connector<TextButton> waterfall_button_connector{&waterfall_button,
                                                            this};
    model::Connector<TextButton> waveform_button_connector{&waveform_button,
                                                           this};
    model::Connector<ToggleButton> follow_playback_connector{
            &follow_playback_button, this};
    model::Connector<ScrollBar> scroll_bar_connector{&scroll_bar, this};
};
