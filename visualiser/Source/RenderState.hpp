#pragma once

#include "ModelWrapper.hpp"

#include "combined/engine.h"

namespace model {

class RenderState {
public:
    bool is_rendering{false};
    engine::State state{engine::State::idle};
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
        is_rendering.set(u.is_rendering, do_notify);
        state.set(u.state, do_notify);
        progress.set(u.progress, do_notify);
        show_waveguide.set(u.show_waveguide, do_notify);
        show_raytracer.set(u.show_raytracer, do_notify);
    }

    void reseat_value(RenderState& u) override {
        is_rendering.reseat(u.is_rendering);
        state.reseat(u.state);
        progress.reseat(u.progress);
        show_waveguide.reseat(u.show_waveguide);
        show_raytracer.reseat(u.show_raytracer);
    }

public:
    ValueWrapper<bool> is_rendering{this, t->is_rendering};
    ValueWrapper<engine::State> state{this, t->state};
    ValueWrapper<double> progress{this, t->progress};
    ValueWrapper<bool> show_waveguide{this, t->show_waveguide};
    ValueWrapper<bool> show_raytracer{this, t->show_raytracer};
};

}  // namespace model