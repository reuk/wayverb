#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class PlaybackViewManager {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void max_range_changed(PlaybackViewManager* r,
                                       const Range<double>& range) = 0;
        virtual void visible_range_changed(PlaybackViewManager* r,
                                           const Range<double>& range) = 0;
        virtual void current_time_changed(PlaybackViewManager* r,
                                          double time) = 0;
    };

    void set_max_range(Range<double> r, bool notify);
    Range<double> get_max_range() const;

    void set_visible_range(Range<double> r, bool notify);
    Range<double> get_visible_range() const;

    void set_follow_playback(bool b);
    bool get_follow_playback() const;

    void set_current_time(double t, bool notify);
    double get_current_time() const;

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    Range<double> max_range;
    Range<double> visible_range;
    bool follow_playback{true};
    double current_time{0};

    ListenerList<Listener> listener_list;
};
