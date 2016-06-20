#include "PlaybackViewManager.hpp"

void PlaybackViewManager::set_max_range(Range<double> r, bool notify) {
    max_range = r;
    if (notify) {
        notify_max_range();
    }
}
void PlaybackViewManager::notify_max_range() {
    listener_list.call(&Listener::max_range_changed, this, max_range);
}
Range<double> PlaybackViewManager::get_max_range() const {
    return max_range;
}

void PlaybackViewManager::set_visible_range(Range<double> r, bool notify) {
    visible_range = max_range.constrainRange(r);
    if (notify) {
        notify_visible_range();
    }
}
void PlaybackViewManager::notify_visible_range() {
    listener_list.call(&Listener::visible_range_changed, this, visible_range);
}
Range<double> PlaybackViewManager::get_visible_range() const {
    return visible_range;
}

void PlaybackViewManager::set_follow_playback(bool b) {
    follow_playback = b;
}
bool PlaybackViewManager::get_follow_playback() const {
    return follow_playback;
}

void PlaybackViewManager::set_current_time(double t) {
    auto current_time = max_range.clipValue(t);
    listener_list.call(&Listener::current_time_changed, this, current_time);

    if (follow_playback && !visible_range.contains(current_time)) {
        set_visible_range(visible_range.movedToStartAt(current_time), true);
    }
}

void PlaybackViewManager::addListener(Listener* l) {
    listener_list.add(l);
}

void PlaybackViewManager::removeListener(Listener* l) {
    listener_list.remove(l);
}

//----------------------------------------------------------------------------//

TransportViewManager::TransportViewManager(
        AudioTransportSource& audio_transport_source)
        : audio_transport_source(audio_transport_source) {
    auto r = Range<double>(0, audio_transport_source.getLengthInSeconds());
    set_max_range(r, true);
    set_visible_range(r, true);
}

void TransportViewManager::reset_view() {
    auto r = Range<double>(0, audio_transport_source.getLengthInSeconds());
    set_max_range(r, true);
    set_visible_range(r, true);
}

void TransportViewManager::timerCallback() {
    set_current_time(audio_transport_source.getCurrentPosition());
}

void TransportViewManager::changeListenerCallback(ChangeBroadcaster* cb) {
    if (cb == &audio_transport_source) {
        if (audio_transport_source.isPlaying()) {
            startTimer(15);
        } else {
            stopTimer();
            timerCallback();
        }
    }
}