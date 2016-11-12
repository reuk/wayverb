#include "PlaybackViewManager.h"

#include "../JuceLibraryCode/JuceHeader.h"

void PlaybackViewManager::set_max_length(double r) {
    max_length = r;
    reset_view();
}

void PlaybackViewManager::set_visible_range(juce::Range<double> r,
                                            bool notify) {
    auto t = juce::Range<double>(0, max_length).constrainRange(r);
    if (visible_range != t) {
        visible_range = t;
        if (notify) {
            notify_visible_range();
        }
    }
}

void PlaybackViewManager::notify_visible_range() {
    listener_list.call(&Listener::visible_range_changed, this, visible_range);
}

double PlaybackViewManager::get_max_length() const {
    return max_length;
}

juce::Range<double> PlaybackViewManager::get_visible_range() const {
    return visible_range;
}

void PlaybackViewManager::reset_view() {
    set_visible_range(juce::Range<double>(0, max_length), true);
}

void PlaybackViewManager::move_to_time(double t) {
    auto current_time = juce::Range<double>(0, max_length).clipValue(t);

    if (!visible_range.contains(current_time)) {
        set_visible_range(visible_range.movedToStartAt(current_time), true);
    }
}

void PlaybackViewManager::addListener(Listener *l) {
    listener_list.add(l);
}

void PlaybackViewManager::removeListener(Listener *l) {
    listener_list.remove(l);
}

//----------------------------------------------------------------------------//

TransportViewManager::TransportViewManager(
        juce::AudioTransportSource &audio_transport_source)
        : audio_transport_source(audio_transport_source)
        , follow_playhead(true) {
    playback_view_manager.addListener(this);

    startTimerHz(60);
}

TransportViewManager::~TransportViewManager() noexcept {
    playback_view_manager.removeListener(this);
}

void TransportViewManager::reset_view() {
    auto l = audio_transport_source.getLengthInSeconds();
    if (l != playback_view_manager.get_max_length()) {
        playback_view_manager.set_max_length(l);
    }
}

void TransportViewManager::move_to_time(double t) {
    playback_view_manager.move_to_time(t);
}

double TransportViewManager::get_max_length() const {
    return playback_view_manager.get_max_length();
}

juce::Range<double> TransportViewManager::get_visible_range() const {
    return playback_view_manager.get_visible_range();
}

void TransportViewManager::set_visible_range(const juce::Range<double> &u,
                                             bool notify) {
    playback_view_manager.set_visible_range(u, notify);
}

void TransportViewManager::visible_range_changed(
        PlaybackViewManager *r, const juce::Range<double> &range) {
    if (r == &playback_view_manager) {
        listener_list.call(&Listener::visible_range_changed, this, range);
    }
}

void TransportViewManager::set_follow_playhead(bool b) {
    follow_playhead = b;
}

bool TransportViewManager::get_follow_playhead() const {
    return follow_playhead;
}

void TransportViewManager::addListener(Listener *l) {
    listener_list.add(l);
}

void TransportViewManager::removeListener(Listener *l) {
    listener_list.remove(l);
}

void TransportViewManager::timerCallback() {
    if (audio_transport_source.isPlaying() && follow_playhead) {
        playback_view_manager.move_to_time(
                audio_transport_source.getCurrentPosition());
    }
    listener_list.call(&Listener::playhead_time_changed,
                       this,
                       audio_transport_source.getCurrentPosition());
}
