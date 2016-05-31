#pragma once

#include "Orientable.hpp"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/fbo.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/render_buffer.h"
#include "modern_gl_utils/screen_quad.h"
#include "modern_gl_utils/texture_object.h"
#include "modern_gl_utils/updatable.h"
#include "modern_gl_utils/vao.h"

class Node : public Orientable {
public:
    using Orientable::Orientable;

    glm::vec3 get_position() const;
    void set_position(const glm::vec3& v);

    glm::vec3 get_scale() const;
    void set_scale(const glm::vec3& s);
    void set_scale(float s);

    glm::mat4 get_matrix() const override;

private:
    glm::vec3 position{0, 0, 0};
    glm::vec3 scale{1, 1, 1};
};

class BasicDrawableObject : public ::Drawable, public Node {
public:
    BasicDrawableObject(const GenericShader& shader,
                        const std::vector<glm::vec3>& g,
                        const std::vector<glm::vec4>& c,
                        const std::vector<GLuint>& i,
                        GLuint mode);

    void draw() const override;

    const GenericShader& get_shader() const;

    GLuint get_mode() const;
    void set_mode(GLuint mode);

private:
    const GenericShader& shader;

    VAO vao;
    StaticVBO geometry;
    StaticVBO colors;
    StaticIBO ibo;
    GLuint size;

    GLuint mode{GL_LINES};
};