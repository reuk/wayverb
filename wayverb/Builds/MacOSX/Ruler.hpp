#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Ruler : public Component {
public:
    class Listener {
    public:
        virtual ~Listener() noexcept = default;

        virtual void ruler_visible_range_changed(Ruler* r,
                                                 const Range<float>& range) = 0;
    };

    Ruler();
    virtual ~Ruler() noexcept;

    void set_max_range(const Range<float>& r);
    void set_visible_range(const Range<float>& r, bool notify);

    float get_time(float x) const;

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
    Range<float> max_range;
    Range<float> visible_range;
    ListenerList<Listener> listener_list;

    struct RulerState;
    std::unique_ptr<RulerState> ruler_state;
};