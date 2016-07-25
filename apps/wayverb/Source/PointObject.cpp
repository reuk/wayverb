#include "PointObject.hpp"

#include <algorithm>
#include <numeric>

namespace {
aligned::vector<glm::vec3> compute_ring_points(int num, RingObject::Axis axis) {
    aligned::vector<glm::vec3> ret(num);
    for (auto i = 0; i != num; ++i) {
        auto angle = M_PI * 2 * i / num;
        switch (axis) {
            case RingObject::Axis::x:
                ret[i] = glm::vec3(sin(angle), cos(angle), 0);
                break;
            case RingObject::Axis::y:
                ret[i] = glm::vec3(0, sin(angle), cos(angle));
                break;
            case RingObject::Axis::z:
                ret[i] = glm::vec3(cos(angle), 0, sin(angle));
                break;
        }
    }
    return ret;
}

glm::vec4 compute_ring_color(RingObject::Axis axis) {
    switch (axis) {
        case RingObject::Axis::x:
            return glm::vec4(1, 0, 0, 1);
        case RingObject::Axis::y:
            return glm::vec4(0, 1, 0, 1);
        case RingObject::Axis::z:
            return glm::vec4(0, 0, 1, 1);
    }
}

aligned::vector<GLuint> compute_ring_indices(int num) {
    aligned::vector<GLuint> ret(num + 1, 0);
    std::iota(ret.begin(), ret.end() - 1, 0);
    return ret;
}
}  // namespace

RingObject::RingObject(mglu::GenericShader& shader,
                       const glm::vec4& color,
                       Axis axis)
        : BasicDrawableObject(shader,
                              compute_ring_points(pts, axis),
                              aligned::vector<glm::vec4>(pts, color),
                              compute_ring_indices(pts),
                              GL_LINE_STRIP) {
    set_scale(0.4);
}

//----------------------------------------------------------------------------//

LineObject::LineObject(mglu::GenericShader& shader, const glm::vec4& color)
        : BasicDrawableObject(shader,
                              {{0, 0, 0}, {0, 0, 1}},
                              aligned::vector<glm::vec4>(2, color),
                              {0, 1},
                              GL_LINES) {
    set_scale(0.4);
}

//----------------------------------------------------------------------------//

PointObject::PointObject(mglu::GenericShader& shader, const glm::vec4& color)
        : shader(&shader)
        , color(color)
        , x_ring(shader, color, RingObject::Axis::x)
        , y_ring(shader, color, RingObject::Axis::y)
        , z_ring(shader, color, RingObject::Axis::z) {
}

void PointObject::set_highlight(float amount) {
    x_ring.set_highlight(amount);
    y_ring.set_highlight(amount);
    z_ring.set_highlight(amount);
}

void PointObject::do_draw(const glm::mat4& modelview_matrix) const {
    x_ring.draw(modelview_matrix);
    y_ring.draw(modelview_matrix);
    z_ring.draw(modelview_matrix);
    for (auto& i : lines) {
        i.draw(modelview_matrix);
    }
}

glm::mat4 PointObject::get_local_modelview_matrix() const {
    return get_matrix();
}

void PointObject::set_pointing(const aligned::vector<glm::vec3>& directions) {
    while (directions.size() < lines.size()) {
        lines.pop_back();
    }
    for (auto i = 0u; i != directions.size(); ++i) {
        if (lines.size() <= i) {
            lines.emplace_back(*shader, color);
        }
        lines[i].set_pointing(directions[i]);
    }
}
