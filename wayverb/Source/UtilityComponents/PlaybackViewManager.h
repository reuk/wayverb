#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class PlaybackViewManager {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener &) = default;
        Listener &operator=(const Listener &) = default;
        Listener(Listener &&) noexcept = default;
        Listener &operator=(Listener &&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void visible_range_changed(
                PlaybackViewManager *r, const juce::Range<double> &range) = 0;
    };

    PlaybackViewManager() = default;
    virtual ~PlaybackViewManager() noexcept = default;

    void set_max_length(double r);
    double get_max_length() const;

    void set_visible_range(juce::Range<double> r, bool notify);
    juce::Range<double> get_visible_range() const;
    void notify_visible_range();

    void reset_view();
    void move_to_time(double t);

    void addListener(Listener *l);
    void removeListener(Listener *l);

private:
    juce::ListenerList<Listener> listener_list;
    double max_length;
    juce::Range<double> visible_range;
};

//----------------------------------------------------------------------------//

class TransportViewManager : private juce::Timer,
                             public PlaybackViewManager::Listener {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener &) = default;
        Listener &operator=(const Listener &) = default;
        Listener(Listener &&) noexcept = default;
        Listener &operator=(Listener &&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void visible_range_changed(
                TransportViewManager *r, const juce::Range<double> &range) = 0;
        virtual void playhead_time_changed(TransportViewManager *r, double t) {}
    };

    TransportViewManager(juce::AudioTransportSource &audio_transport_source);
    virtual ~TransportViewManager() noexcept;

    void reset_view();
    void move_to_time(double t);

    double get_max_length() const;
    juce::Range<double> get_visible_range() const;
    void set_visible_range(const juce::Range<double> &u, bool notify);

    void visible_range_changed(PlaybackViewManager *r,
                               const juce::Range<double> &range) override;

    void set_follow_playhead(bool b);
    bool get_follow_playhead() const;

    void addListener(Listener *l);
    void removeListener(Listener *l);

private:
    void timerCallback() override;

    juce::AudioTransportSource &audio_transport_source;
    juce::ListenerList<Listener> listener_list;
    PlaybackViewManager playback_view_manager;
    bool follow_playhead;
};
