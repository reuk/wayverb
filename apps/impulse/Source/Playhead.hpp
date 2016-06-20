#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Playhead : public Component {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void playhead_dragged(Playhead*, const MouseEvent& e) = 0;
    };

    void paint(Graphics& g) override;

    void mouseEnter(const MouseEvent& e) override;
    void mouseExit(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    ListenerList<Listener> listener_list;
};