#pragma once

#include "BasicDrawableObject.hpp"

class RingObject : public BasicDrawableObject {
public:
    enum class Axis { x, y, z };
    static constexpr auto pts = 24;
    RingObject(mglu::GenericShader& shader, const glm::vec4& color, Axis axis);
};

class LineObject : public BasicDrawableObject {
public:
    LineObject(mglu::GenericShader& shader, const glm::vec4& color);
    LineObject(LineObject&&) noexcept = default;
    LineObject& operator=(LineObject&&) noexcept = default;
};

class PointObject : public mglu::Drawable, public Node {
public:
    PointObject(mglu::GenericShader& shader, const glm::vec4& color);

    void set_pointing(const std::vector<glm::vec3>& directions);

    void set_highlight(float amount);

private:
    void do_draw(const glm::mat4& modelview_matrix) const;
    glm::mat4 get_local_modelview_matrix() const override;

    mglu::GenericShader* shader;
    glm::vec4 color;

    RingObject x_ring;
    RingObject y_ring;
    RingObject z_ring;

    std::vector<LineObject> lines;
};