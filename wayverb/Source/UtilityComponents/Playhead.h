#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Playhead : public juce::Component {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void playhead_dragged(Playhead*, const juce::MouseEvent& e) = 0;
    };

    void paint(juce::Graphics& g) override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    juce::ListenerList<Listener> listener_list;
};
