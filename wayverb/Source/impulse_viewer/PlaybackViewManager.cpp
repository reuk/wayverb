#include "PlaybackViewManager.hpp"

void PlaybackViewManager::set_max_range(Range<double> r, bool notify) {
    max_range = r;
    if (notify) {
        listener_list.call(&Listener::max_range_changed, this, max_range);
    }
}
Range<double> PlaybackViewManager::get_max_range() const {
    return max_range;
}

void PlaybackViewManager::set_visible_range(Range<double> r, bool notify) {
    visible_range = max_range.constrainRange(r);
    if (notify) {
        listener_list.call(
                &Listener::visible_range_changed, this, visible_range);
    }
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

TransportViewManager::TransportViewManager() {}

void TransportViewManager::timerCallback() {
    set_current_time(getCurrentPosition());
}

void TransportViewManager::setPosition(double t) {
    AudioTransportSource::setPosition(t);
    timerCallback();
}

void TransportViewManager::start() {
    startTimer(15);
    AudioTransportSource::start();
}

void TransportViewManager::stop() {
    stopTimer();
    AudioTransportSource::stop();
}