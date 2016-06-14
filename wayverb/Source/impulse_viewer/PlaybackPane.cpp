#include "PlaybackPane.hpp"

#include "CommandIDs.h"
#include "Main.hpp"

PlaybackPane::PlaybackPane(AudioDeviceManager& audio_device_manager,
                           AudioFormatManager& audio_format_manager,
                           AudioFormatReaderSource& audio_format_reader_source,
                           const File& file)
        : audio_device_manager(audio_device_manager)
        , audio_format_manager(audio_format_manager)
        , audio_format_reader_source(audio_format_reader_source)
        , renderer(audio_format_manager, playback_view_manager)
        , ruler(playback_view_manager)
        , load_different_button("load different file")
        , follow_playback_button("follow playback")
        , scroll_bar(false)
        , transport(playback_view_manager) {
    playback_view_manager.setSource(
            &audio_format_reader_source,
            0,
            nullptr,
            audio_format_reader_source.getAudioFormatReader()->sampleRate);
    audio_source_player.setSource(&playback_view_manager);
    audio_device_manager.addAudioCallback(&audio_source_player);

    renderer.set_reader(audio_format_manager.createReaderFor(file), 0);

    auto& command_manager = VisualiserApplication::get_command_manager();
    command_manager.registerAllCommandsForTarget(this);

    auto r = Range<double>(0, playback_view_manager.getLengthInSeconds());
    playback_view_manager.set_max_range(r, true);
    playback_view_manager.set_visible_range(r, true);

    scroll_bar.setAutoHide(false);

    follow_playback_button.setWantsKeyboardFocus(false);
    follow_playback_button.setToggleState(true, true);

    addAndMakeVisible(renderer);
    addAndMakeVisible(follow_playback_button);
    addAndMakeVisible(scroll_bar);
    addAndMakeVisible(transport);
    addAndMakeVisible(ruler);
    addAndMakeVisible(load_different_button);

    setWantsKeyboardFocus(true);
}

PlaybackPane::~PlaybackPane() noexcept {
    audio_device_manager.removeAudioCallback(&audio_source_player);
}

void PlaybackPane::resized() {
    auto bounds = getLocalBounds();

    scroll_bar.setBounds(bounds.removeFromBottom(20));

    auto top = bounds.removeFromTop(40);
    top.reduce(2, 2);
    ruler.setBounds(bounds.removeFromTop(20));
    renderer.setBounds(bounds);

    transport.setBounds(top.removeFromLeft(200));
    follow_playback_button.setBounds(top.removeFromRight(150));

    load_different_button.setBounds(top.removeFromLeft(150));
}

void PlaybackPane::max_range_changed(PlaybackViewManager* r,
                                     const Range<double>& range) {
    scroll_bar.setRangeLimits(range);
}

void PlaybackPane::visible_range_changed(PlaybackViewManager* r,
                                         const Range<double>& range) {
    scroll_bar.setCurrentRange(range, dontSendNotification);
}

void PlaybackPane::current_time_changed(PlaybackViewManager* r, double time) {
}

void PlaybackPane::scrollBarMoved(ScrollBar* s, double new_range_start) {
    if (s == &scroll_bar) {
        playback_view_manager.set_visible_range(scroll_bar.getCurrentRange(),
                                                true);
    }
}

void PlaybackPane::getAllCommands(Array<CommandID>& commands) {
    commands.addArray({CommandIDs::idPlay,
                       CommandIDs::idPause,
                       CommandIDs::idReturnToBeginning});
}
void PlaybackPane::getCommandInfo(CommandID command_id,
                                  ApplicationCommandInfo& result) {
    switch (command_id) {
        case CommandIDs::idPlay:
            result.setInfo("Play", "Start playback", "General", 0);
            result.defaultKeypresses.add(
                    KeyPress(KeyPress::spaceKey, ModifierKeys::noModifiers, 0));
            result.setActive(!playback_view_manager.isPlaying());
            break;

        case CommandIDs::idPause:
            result.setInfo("Pause", "Pause playback", "General", 0);
            result.defaultKeypresses.add(
                    KeyPress(KeyPress::spaceKey, ModifierKeys::noModifiers, 0));
            result.setActive(playback_view_manager.isPlaying());
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
bool PlaybackPane::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idPlay:
            playback_view_manager.start();
            VisualiserApplication::get_command_manager().commandStatusChanged();
            return true;

        case CommandIDs::idPause:
            playback_view_manager.stop();
            VisualiserApplication::get_command_manager().commandStatusChanged();
            return true;

        case CommandIDs::idReturnToBeginning:
            playback_view_manager.setPosition(0);
            return true;

        default:
            return false;
    }
}
ApplicationCommandTarget* PlaybackPane::getNextCommandTarget() {
    return findFirstTargetParentComponent();
}

void PlaybackPane::buttonClicked(Button* b) {
    if (b == &follow_playback_button) {
        playback_view_manager.set_follow_playback(
                follow_playback_button.getToggleState());
    } else if (b == &load_different_button) {
        FileChooser fc("open...",
                       File::nonexistent,
                       audio_format_manager.getWildcardForAllFormats());
        if (fc.browseForFileToOpen()) {
            listener_list.call(
                    &FileDropListener::file_dropped, this, fc.getResult());
        }
    }
}

void PlaybackPane::addListener(FileDropListener* f) {
    listener_list.add(f);
}

void PlaybackPane::removeListener(FileDropListener* f) {
    listener_list.remove(f);
}