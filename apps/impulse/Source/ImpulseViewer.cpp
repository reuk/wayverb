#include "ImpulseViewer.hpp"
#include "Application.hpp"
#include "CommandIDs.hpp"

#include <sstream>

ImpulseViewer::ImpulseViewer(AudioDeviceManager& audio_device_manager,
                             AudioFormatManager& audio_format_manager,
                             const File& file)
        : audio_device_manager(audio_device_manager)
        , audio_format_reader_source(audio_format_manager.createReaderFor(file),
                                     true)
        , transport_view_manager(audio_transport_source)
        , renderer(audio_transport_source, audio_format_manager, file)
        , ruler(transport_view_manager)
        , waterfall_button("waterfall")
        , waveform_button("waveform")
        , follow_playback_button("follow playback")
        , scroll_bar(false)
        , transport(audio_transport_source) {
    auto& command_manager = ImpulseApplication::get_command_manager();
    command_manager.registerAllCommandsForTarget(this);

    audio_transport_source.setSource(
            &audio_format_reader_source,
            0,
            nullptr,
            audio_format_reader_source.getAudioFormatReader()->sampleRate);
    transport_view_manager.reset_view();

    audio_source_player.setSource(&audio_transport_source);
    audio_device_manager.addAudioCallback(&audio_source_player);

    for (auto i : {&waterfall_button, &waveform_button}) {
        i->setRadioGroupId(0xf);
        i->setClickingTogglesState(true);
        i->setWantsKeyboardFocus(false);
    }

    waveform_button.setToggleState(true, sendNotification);

    follow_playback_button.setWantsKeyboardFocus(false);
    follow_playback_button.triggerClick();

    auto num_channels =
            audio_format_reader_source.getAudioFormatReader()->numChannels;
    for (auto i = 0u; i != num_channels; ++i) {
        auto channel = i + 1;
        std::stringstream ss;
        ss << "channel " << channel;
        channel_combo_box.addItem(ss.str(), channel);
    }
    channel_combo_box.setSelectedItemIndex(0);

    addAndMakeVisible(renderer);
    addAndMakeVisible(ruler);
    addAndMakeVisible(channel_combo_box);
    addAndMakeVisible(waterfall_button);
    addAndMakeVisible(waveform_button);
    addAndMakeVisible(follow_playback_button);
    addAndMakeVisible(scroll_bar);
    addAndMakeVisible(transport);

    setWantsKeyboardFocus(true);

    visible_range_changed(&transport_view_manager,
                          transport_view_manager.get_visible_range());
}

ImpulseViewer::~ImpulseViewer() noexcept {
    audio_device_manager.removeAudioCallback(&audio_source_player);
}

void ImpulseViewer::visible_range_changed(TransportViewManager* r, const juce::Range<double>& range) {
    scroll_bar.setRangeLimits(0, transport_view_manager.get_max_length());
    scroll_bar.setCurrentRange(range, dontSendNotification);
    renderer.get_renderer().set_visible_range(range);
}

void ImpulseViewer::playhead_time_changed(TransportViewManager* r, double t) {}

void ImpulseViewer::scrollBarMoved(ScrollBar* s, double new_range_start) {
    if (s == &scroll_bar) {
        transport_view_manager.set_visible_range(scroll_bar.getCurrentRange(),
                                                 true);
    }
}

void ImpulseViewer::resized() {
    scroll_bar.setBounds(getLocalBounds().removeFromBottom(20));

    auto bounds = getLocalBounds();
    auto top = bounds.removeFromTop(40);
    top.reduce(2, 2);
    ruler.setBounds(bounds.removeFromTop(20));
    renderer.setBounds(bounds);

    transport.setBounds(top.removeFromLeft(200));

    follow_playback_button.setBounds(top.removeFromRight(150));

    waveform_button.setBounds(top.removeFromLeft(100));
    top.removeFromLeft(2);
    waterfall_button.setBounds(top.removeFromLeft(100));

    top.reduce(2, 0);

    channel_combo_box.setBounds(top);
}

void ImpulseViewer::getAllCommands(Array<CommandID>& commands) {
    commands.addArray({CommandIDs::idPlay,
                       CommandIDs::idPause,
                       CommandIDs::idReturnToBeginning});
}
void ImpulseViewer::getCommandInfo(CommandID command_id,
                                   ApplicationCommandInfo& result) {
    switch (command_id) {
        case CommandIDs::idPlay:
            result.setInfo("Play", "Start playback", "General", 0);
            result.defaultKeypresses.add(
                    KeyPress(KeyPress::spaceKey, ModifierKeys::noModifiers, 0));
            result.setActive(!audio_transport_source.isPlaying());
            break;

        case CommandIDs::idPause:
            result.setInfo("Pause", "Pause playback", "General", 0);
            result.defaultKeypresses.add(
                    KeyPress(KeyPress::spaceKey, ModifierKeys::noModifiers, 0));
            result.setActive(audio_transport_source.isPlaying());
            break;

        case CommandIDs::idReturnToBeginning:
            result.setInfo("Go to Start",
                           "Move playhead to the beginning",
                           "General",
                           0);
            result.defaultKeypresses.add(KeyPress(
                    KeyPress::returnKey, ModifierKeys::noModifiers, 0));
            break;

        default:
            break;
    }
}
bool ImpulseViewer::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idPlay:
            audio_transport_source.start();
            ImpulseApplication::get_command_manager().commandStatusChanged();
            return true;

        case CommandIDs::idPause:
            audio_transport_source.stop();
            ImpulseApplication::get_command_manager().commandStatusChanged();
            return true;

        case CommandIDs::idReturnToBeginning:
            audio_transport_source.setPosition(0);
            return true;

        default:
            return false;
    }
}
ApplicationCommandTarget* ImpulseViewer::getNextCommandTarget() {
    return findFirstTargetParentComponent();
}

void ImpulseViewer::buttonClicked(Button* b) {
    if (b == &waterfall_button) {
        renderer.get_renderer().set_mode(ImpulseRenderer::Mode::waterfall);
    } else if (b == &waveform_button) {
        renderer.get_renderer().set_mode(ImpulseRenderer::Mode::waveform);
    } else if (b == &follow_playback_button) {
        transport_view_manager.set_follow_playhead(
                follow_playback_button.getToggleState());
    }
}

void ImpulseViewer::comboBoxChanged(ComboBox* c) {
    if (c == &channel_combo_box) {
        renderer.get_renderer().set_channel(
                channel_combo_box.getSelectedItemIndex());
    }
}