#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

enum class RenderState {
    stopped,
    started,
};

class RenderStateManager {
public:
    class Listener {
    public:
        Listener() = default;
        virtual ~Listener() noexcept = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) = default;
        Listener& operator=(Listener&&) = default;

        virtual void render_state_changed(RenderStateManager*,
                                          RenderState state) = 0;
        virtual void render_progress_changed(RenderStateManager*,
                                             double progress) = 0;
    };

    double get_progress() const;
    void set_progress(double progress);

    RenderState get_state() const;
    void set_state(RenderState state);

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    RenderState state{RenderState::stopped};
    double progress{0};

    ListenerList<Listener> listener_list;
};
