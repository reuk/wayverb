#include "RenderHelpers.hpp"

void BaseContextLifetime::set_viewport(const glm::vec2& v) {
    viewport = v;
}

glm::vec2 BaseContextLifetime::get_viewport() const {
    return viewport;
}

float BaseContextLifetime::get_aspect() const {
    return get_viewport().x / get_viewport().y;
}

//----------------------------------------------------------------------------//

void BaseRenderer::set_viewport(const glm::vec2& u) {
    incoming_work_queue.push([this, u] {
        if (get_context_lifetime()) {
            get_context_lifetime()->set_viewport(u);
        }
    });
}

void BaseRenderer::newOpenGLContextCreated() {
    push_outgoing_impl([this] { sendChangeMessage(); });
}

void BaseRenderer::renderOpenGL() {
    while (!incoming_work_queue.empty()) {
        if (auto method = incoming_work_queue.pop()) {
            (*method)();
        }
    }
    get_context_lifetime()->update(0);
    get_context_lifetime()->draw();
}

void BaseRenderer::openGLContextClosing() {
    push_outgoing_impl([this] { sendChangeMessage(); });
}

void BaseRenderer::handleAsyncUpdate() {
    while (!outgoing_work_queue.empty()) {
        if (auto method = outgoing_work_queue.pop()) {
            (*method)();
        };
    }
}
