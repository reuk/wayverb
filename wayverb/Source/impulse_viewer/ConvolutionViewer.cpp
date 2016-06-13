#include "ConvolutionViewer.hpp"

#include "CommandIDs.h"
#include "Main.hpp"
#include "lerp.h"

AudioThumbnailPane::AudioThumbnailPane(
        AudioFormatManager& audio_format_manager,
        PlaybackViewManager& playback_view_manager)
        : audio_transport_source(audio_transport_source)
        , playback_view_manager(playback_view_manager)
        , audio_thumbnail_cache(16)
        , audio_thumbnail(256, audio_format_manager, audio_thumbnail_cache) {
    addAndMakeVisible(playhead);
}

void AudioThumbnailPane::paint(Graphics& g) {
    g.fillAll(Colours::black);
    auto num_channels = audio_thumbnail.getNumChannels();
    for (auto i = 0; i != num_channels; ++i) {
        auto start = i * getHeight() / num_channels;
        g.setFillType(
                FillType(ColourGradient(Colours::white,
                                        0,
                                        start,
                                        Colours::darkgrey,
                                        0,
                                        start + getHeight() / num_channels,
                                        false)));
        audio_thumbnail.drawChannel(
                g,
                Rectangle<int>(
                        0, start, getWidth(), getHeight() / num_channels),
                playback_view_manager.get_visible_range().getStart(),
                playback_view_manager.get_visible_range().getEnd(),
                i,
                1.0);
    }
}

void AudioThumbnailPane::resized() {
    playhead.setSize(1, getHeight());
    position_playhead();
    repaint();
}

void AudioThumbnailPane::changeListenerCallback(ChangeBroadcaster* cb) {
    position_playhead();
    repaint();
}

void AudioThumbnailPane::set_reader(AudioFormatReader* new_reader, int64 hash) {
    audio_thumbnail.setReader(new_reader, hash);
}

double AudioThumbnailPane::time_to_x(double t) const {
    return lerp(t,
                playback_view_manager.get_visible_range(),
                Range<double>(0, getWidth()));
}

void AudioThumbnailPane::position_playhead() {
    playhead.setTopLeftPosition(
            time_to_x(playback_view_manager.get_current_time()), 0);
}

void AudioThumbnailPane::max_range_changed(PlaybackViewManager* r,
                                           const Range<double>& range) {
    position_playhead();
    repaint();
}
void AudioThumbnailPane::visible_range_changed(PlaybackViewManager* r,
                                               const Range<double>& range) {
    position_playhead();
    repaint();
}
void AudioThumbnailPane::current_time_changed(PlaybackViewManager* r,
                                              double time) {
    position_playhead();
    repaint();
}

//----------------------------------------------------------------------------//

ConvolutionViewer::ConvolutionViewer(AudioDeviceManager& audio_device_manager,
                                     AudioFormatManager& audio_format_manager,
                                     const File& file)
        : audio_device_manager(audio_device_manager)
        , audio_format_reader_source(audio_format_manager.createReaderFor(file),
                                     true)
        , renderer(audio_format_manager, playback_view_manager)
        , ruler(playback_view_manager)
        , follow_playback_button("follow playback")
        , scroll_bar(false)
        , transport(audio_transport_source) {
    audio_transport_source.setSource(
            &audio_format_reader_source,
            0,
            nullptr,
            audio_format_reader_source.getAudioFormatReader()->sampleRate);
    audio_source_player.setSource(&audio_transport_source);
    audio_device_manager.addAudioCallback(&audio_source_player);

    renderer.set_reader(audio_format_manager.createReaderFor(file), 0);

    auto& command_manager = VisualiserApplication::get_command_manager();
    command_manager.registerAllCommandsForTarget(this);

    auto r = Range<double>(0, audio_transport_source.getLengthInSeconds());
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

    setWantsKeyboardFocus(true);
}

ConvolutionViewer::~ConvolutionViewer() noexcept {
    audio_device_manager.removeAudioCallback(&audio_source_player);
}

void ConvolutionViewer::resized() {
    auto bounds = getLocalBounds();

    scroll_bar.setBounds(bounds.removeFromBottom(20));

    auto top = bounds.removeFromTop(40);
    top.reduce(2, 2);
    ruler.setBounds(bounds.removeFromTop(20));
    renderer.setBounds(bounds);

    transport.setBounds(top.removeFromLeft(200));
    follow_playback_button.setBounds(top.removeFromRight(150));
}

void ConvolutionViewer::max_range_changed(PlaybackViewManager* r,
                                          const Range<double>& range) {
    scroll_bar.setRangeLimits(range);
}

void ConvolutionViewer::visible_range_changed(PlaybackViewManager* r,
                                              const Range<double>& range) {
    scroll_bar.setCurrentRange(range, dontSendNotification);
}

void ConvolutionViewer::current_time_changed(PlaybackViewManager* r,
                                             double time) {
}

void ConvolutionViewer::scrollBarMoved(ScrollBar* s, double new_range_start) {
    if (s == &scroll_bar) {
        playback_view_manager.set_visible_range(scroll_bar.getCurrentRange(),
                                                true);
    }
}

void ConvolutionViewer::timerCallback() {
    playback_view_manager.set_current_time(
            audio_transport_source.getCurrentPosition(), true);
}

void ConvolutionViewer::getAllCommands(Array<CommandID>& commands) {
    commands.addArray({CommandIDs::idPlay,
                       CommandIDs::idPause,
                       CommandIDs::idReturnToBeginning});
}
void ConvolutionViewer::getCommandInfo(CommandID command_id,
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
bool ConvolutionViewer::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idPlay:
            audio_transport_source.start();
            VisualiserApplication::get_command_manager().commandStatusChanged();
            startTimer(15);
            return true;

        case CommandIDs::idPause:
            audio_transport_source.stop();
            VisualiserApplication::get_command_manager().commandStatusChanged();
            stopTimer();
            timerCallback();
            return true;

        case CommandIDs::idReturnToBeginning:
            audio_transport_source.setPosition(0);
            timerCallback();
            return true;

        default:
            return false;
    }
}
ApplicationCommandTarget* ConvolutionViewer::getNextCommandTarget() {
    return findFirstTargetParentComponent();
}

void ConvolutionViewer::buttonClicked(Button* b) {
    if (b == &follow_playback_button) {
        playback_view_manager.set_follow_playback(
                follow_playback_button.getToggleState());
    }
}