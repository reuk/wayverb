#include "OtherComponents/RenderHelpers.hpp"

void BaseContextLifetime::set_viewport(const glm::vec2 &v) {
    viewport = v;
    viewport_changed(viewport);
}

glm::vec2 BaseContextLifetime::get_viewport() const {
    return viewport;
}

float BaseContextLifetime::get_aspect() const {
    return get_viewport().x / get_viewport().y;
}

void BaseContextLifetime::viewport_changed(const glm::vec2 &v) {
}

//----------------------------------------------------------------------------//

void BaseRenderer::set_viewport(const glm::vec2 &u) {
    viewport = u;
    notify_viewport_impl();
}

void BaseRenderer::notify_viewport_impl() {
    incoming_work_queue.push([this] {
        if (get_context_lifetime()) {
            get_context_lifetime()->set_viewport(viewport);
        }
    });
}

void BaseRenderer::newOpenGLContextCreated() {
    notify_viewport_impl();
    push_outgoing_impl([this] { sendChangeMessage(); });
}

void BaseRenderer::renderOpenGL() {
    while (auto method = incoming_work_queue.pop()) {
        (*method)();
    }
    get_context_lifetime()->update(0);
    get_context_lifetime()->draw(glm::mat4{});
}

void BaseRenderer::openGLContextClosing() {
    push_outgoing_impl([this] { sendChangeMessage(); });
}

void BaseRenderer::handleAsyncUpdate() {
    while (auto method = outgoing_work_queue.pop()) {
        (*method)();
    }
}

void BaseRenderer::mouse_down(const glm::vec2 &u) {
    push_incoming([this, u] { get_context_lifetime()->mouse_down(u); });
}

void BaseRenderer::mouse_drag(const glm::vec2 &u) {
    push_incoming([this, u] { get_context_lifetime()->mouse_drag(u); });
}

void BaseRenderer::mouse_up(const glm::vec2 &u) {
    push_incoming([this, u] { get_context_lifetime()->mouse_up(u); });
}

void BaseRenderer::mouse_wheel_move(float u) {
    push_incoming([this, u] { get_context_lifetime()->mouse_wheel_move(u); });
}
