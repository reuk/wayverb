#include "RenderState.hpp"

double RenderStateManager::get_progress() const {
    return progress;
}

void RenderStateManager::set_progress(double x) {
    progress = x;
    listener_list.call(
        &Listener::render_progress_changed, this, get_progress());
}

RenderState RenderStateManager::get_state() const {
    return state;
}

void RenderStateManager::set_state(RenderState x) {
    state = x;
    listener_list.call(&Listener::render_state_changed, this, get_state());
}

void RenderStateManager::addListener(Listener* listener) {
    listener_list.add(listener);
}
void RenderStateManager::removeListener(Listener* listener) {
    listener_list.remove(listener);
}
