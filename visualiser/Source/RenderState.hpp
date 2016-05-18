#pragma once

#include "ModelWrapper.hpp"

namespace model {

class RenderState {
public:
    enum class State {
        stopped,
        started,
    };

    State state{State::stopped};
    double progress{0};
    bool show_waveguide{true};
    bool show_raytracer{true};
};

template <>
class ValueWrapper<RenderState> : public NestedValueWrapper<RenderState> {
public:
    using NestedValueWrapper<RenderState>::NestedValueWrapper;

protected:
    void set_value(const RenderState& u, bool do_notify = true) override {
        state.set(u.state, do_notify);
        progress.set(u.progress, do_notify);
        show_waveguide.set(u.show_waveguide, do_notify);
        show_raytracer.set(u.show_raytracer, do_notify);
    }

    void reseat_value(RenderState& u) override {
        state.reseat(u.state);
        progress.reseat(u.progress);
        show_waveguide.reseat(u.show_waveguide);
        show_raytracer.reseat(u.show_raytracer);
    }

public:
    ValueWrapper<RenderState::State> state{this, t->state};
    ValueWrapper<double> progress{this, t->progress};
    ValueWrapper<bool> show_waveguide{this, t->show_waveguide};
    ValueWrapper<bool> show_raytracer{this, t->show_raytracer};
};

}  // namespace model