#pragma once

#include "ModelWrapper.hpp"

namespace model {

enum class RenderState {
    stopped,
    started,
};

class RenderStateManager {
public:
    RenderState state{RenderState::stopped};
    double progress{0};
};

template <>
class ValueWrapper<RenderStateManager>
    : public NestedValueWrapper<RenderStateManager> {
public:
    using NestedValueWrapper<RenderStateManager>::NestedValueWrapper;

    void set_value(const RenderStateManager& u,
                   bool do_notify = true) override {
        state.set_value(u.state, do_notify);
        progress.set_value(u.progress, do_notify);
    }

    void reseat(RenderStateManager& u) override {
        state.reseat(u.state);
        progress.reseat(u.progress);
    }

    ValueWrapper<RenderState> state{this, t->state};
    ValueWrapper<double> progress{this, t->progress};
};

}  // namespace model