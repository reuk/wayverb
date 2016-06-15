#include "AudioThumbnailPane.hpp"

#include "lerp.h"

AudioThumbnailPane::AudioThumbnailPane(
        AudioFormatManager& audio_format_manager,
        TransportViewManager& transport_view_manager)
        : transport_view_manager(transport_view_manager)
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
                transport_view_manager.get_visible_range().getStart(),
                transport_view_manager.get_visible_range().getEnd(),
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

void AudioThumbnailPane::playhead_dragged(Playhead* p, const MouseEvent& e) {
    if (p == &playhead &&
        !transport_view_manager.audio_transport_source.isPlaying()) {
        auto mouse_pos = e.getEventRelativeTo(this).getPosition().getX();
        auto constrained = Range<int>(1, getWidth() - 1).clipValue(mouse_pos);
        transport_view_manager.audio_transport_source.setPosition(
                x_to_time(constrained));
        transport_view_manager.timerCallback();
    }
}

double AudioThumbnailPane::time_to_x(double t) const {
    return lerp(t,
                transport_view_manager.get_visible_range(),
                Range<double>(0, getWidth()));
}
double AudioThumbnailPane::x_to_time(double t) const {
    return lerp(t,
                Range<double>(0, getWidth()),
                transport_view_manager.get_visible_range());
}
void AudioThumbnailPane::position_playhead() {
    playhead.setTopLeftPosition(
            time_to_x(transport_view_manager.audio_transport_source
                              .getCurrentPosition()),
            0);
}
