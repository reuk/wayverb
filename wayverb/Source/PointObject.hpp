#pragma once

#include "BasicDrawableObject.hpp"

class RingObject : public BasicDrawableObject {
public:
    enum class Axis { x, y, z };
    static constexpr auto pts = 24;
    RingObject(const GenericShader& shader, const glm::vec4& color, Axis axis);
};

class LineObject : public BasicDrawableObject {
public:
    LineObject(const GenericShader& shader, const glm::vec4& color);
};

class PointObject : public ::Drawable, public Node {
public:
    PointObject(const GenericShader& shader, const glm::vec4& color);

    void draw() const override;

    void set_position(const glm::vec3& p);
    void set_pointing(const std::vector<glm::vec3>& directions);

private:
    const GenericShader& shader;
    glm::vec4 color;

    RingObject x_ring;
    RingObject y_ring;
    RingObject z_ring;

    std::vector<LineObject> lines;
};