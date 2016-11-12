#pragma once

#include "PlaybackViewManager.h"
#include "Playhead.h"

class AudioThumbnailPane : public juce::Component,
                           public juce::ChangeListener,
                           public TransportViewManager::Listener,
                           public Playhead::Listener {
public:
    AudioThumbnailPane(juce::AudioTransportSource& audio_transport_source,
                       juce::AudioFormatManager& audio_format_manager,
                       TransportViewManager& transport_view_manager);
    virtual ~AudioThumbnailPane() noexcept;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster* cb) override;

    void set_reader(juce::AudioFormatReader* new_reader, juce::int64 hash);

    void visible_range_changed(TransportViewManager* r,
                               const juce::Range<double>& range) override;
    void playhead_time_changed(TransportViewManager *r, double t) override;

    void playhead_dragged(Playhead* p, const juce::MouseEvent& e) override;

private:
    double time_to_x(double t) const;
    double x_to_time(double t) const;
    void position_playhead();

    juce::AudioTransportSource& audio_transport_source;
    TransportViewManager& transport_view_manager;

    juce::AudioThumbnailCache audio_thumbnail_cache;
    juce::AudioThumbnail audio_thumbnail;
    Playhead playhead;
};
