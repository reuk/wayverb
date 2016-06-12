#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Ruler : public Component {
public:
    class Listener {
    public:
        virtual ~Listener() noexcept = default;

        virtual void ruler_visible_range_changed(
                Ruler* r, const Range<double>& range) = 0;
    };

    Ruler();
    virtual ~Ruler() noexcept;

    void set_max_range(const Range<double>& r);
    void set_visible_range(const Range<double>& r, bool notify);

    void set_follow_playback(bool follow);
    bool get_follow_playback() const;

    void set_current_time(double t);
    double get_current_time() const;

    double get_time(double x) const;
    double get_x(double time) const;

    void paint(Graphics& g) override;

    void mouseEnter(const MouseEvent& event) override;
    void mouseExit(const MouseEvent& event) override;
    void mouseDown(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseDoubleClick(const MouseEvent& event) override;

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    bool follow_playback{true};
    double current_time{0};

    Range<double> max_range;
    Range<double> visible_range;
    ListenerList<Listener> listener_list;

    struct RulerState;
    std::unique_ptr<RulerState> ruler_state;
};