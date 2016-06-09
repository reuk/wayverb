#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Ruler : public Component {
public:
    class Listener {
    public:
        virtual ~Listener() noexcept = default;
        virtual void rulerMouseDown(Ruler* ruler,
                                    const MouseEvent& e,
                                    float time) = 0;
        virtual void rulerMouseUp(Ruler* ruler, const MouseEvent& e) = 0;
        virtual void rulerDragged(Ruler* ruler, const MouseEvent& e) = 0;
        virtual void rulerDoubleClicked(Ruler* ruler, const MouseEvent& e) = 0;
    };

    Ruler();
    virtual ~Ruler() noexcept = default;

    void setMaximumRange(double max);

    void paint(Graphics& g) override;
    void resized() override;

    void mouseEnter(const MouseEvent& event) override;
    void mouseExit(const MouseEvent& event) override;
    void mouseDown(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseDoubleClick(const MouseEvent& event) override;

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    double max{0};
    ListenerList<Listener> listenerList;
};