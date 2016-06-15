#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "connector.h"

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

    PlaybackViewManager() = default;
    virtual ~PlaybackViewManager() noexcept = default;

    void set_max_range(Range<double> r, bool notify);
    Range<double> get_max_range() const;
    void notify_max_range();

    void set_visible_range(Range<double> r, bool notify);
    Range<double> get_visible_range() const;
    void notify_visible_range();

    void set_follow_playback(bool b);
    bool get_follow_playback() const;

    void addListener(Listener* l);
    void removeListener(Listener* l);

protected:
    void set_current_time(double t);

private:
    Range<double> max_range;
    Range<double> visible_range;
    bool follow_playback{true};

    ListenerList<Listener> listener_list;
};

//----------------------------------------------------------------------------//

class TransportViewManager : public PlaybackViewManager,
                             public Timer,
                             public ChangeListener {
public:
    TransportViewManager(AudioTransportSource& audio_transport_source);

    void reset_view();

    void changeListenerCallback(ChangeBroadcaster* cb) override;
    void timerCallback() override;

    void trigger();

    AudioTransportSource& audio_transport_source;
    model::Connector<ChangeBroadcaster, ChangeListener> source_connector{
            &audio_transport_source, this};
};
