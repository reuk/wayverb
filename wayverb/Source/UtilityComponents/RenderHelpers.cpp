#include "RenderHelpers.hpp"

void BaseContextLifetime::set_viewport(const glm::vec2 &v) {
    viewport = v;
    viewport_changed(viewport);
}

glm::vec2 BaseContextLifetime::get_viewport() const { return viewport; }

float BaseContextLifetime::get_aspect() const {
    return get_viewport().x / get_viewport().y;
}

void BaseContextLifetime::viewport_changed(const glm::vec2 &v) {}

