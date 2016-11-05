#pragma once

#include "BasicDrawableObject.hpp"

class RingObject : public BasicDrawableObject {
public:
    enum class Axis { x, y, z };
    static constexpr auto pts = 24;
    RingObject(const std::shared_ptr<mglu::generic_shader>& shader,
               const glm::vec4& color,
               Axis axis);
};

class LineObject : public BasicDrawableObject {
public:
    LineObject(const std::shared_ptr<mglu::generic_shader>& shader,
               const glm::vec4& color);
    LineObject(LineObject&&) noexcept = default;
    LineObject& operator=(LineObject&&) noexcept = default;
};

class PointObject : public mglu::drawable, public node {
public:
    PointObject(const std::shared_ptr<mglu::generic_shader>& shader,
                const glm::vec4& color);

    void set_pointing(const util::aligned::vector<glm::vec3>& directions);

    void set_highlight(float amount);

private:
    void do_draw(const glm::mat4& modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    std::shared_ptr<mglu::generic_shader> shader;
    glm::vec4 color;

    RingObject x_ring;
    RingObject y_ring;
    RingObject z_ring;

    util::aligned::vector<LineObject> lines;
};
