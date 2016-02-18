#pragma once

#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"
#include "modern_gl_utils/vao.h"
#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/fbo.h"
#include "modern_gl_utils/texture_object.h"
#include "modern_gl_utils/render_buffer.h"
#include "modern_gl_utils/screen_quad.h"

class BasicDrawableObject : public ::Drawable {
public:
    BasicDrawableObject(const GenericShader& shader,
                        const std::vector<glm::vec3>& g,
                        const std::vector<glm::vec4>& c,
                        const std::vector<GLuint>& i,
                        GLuint DRAW_MODE);

    void draw() const override;

    glm::vec3 get_position() const;
    void set_position(const glm::vec3& p);

    glm::vec3 get_scale() const;
    void set_scale(const glm::vec3& s);
    void set_scale(float s);

    const GenericShader& get_shader() const;

private:
    glm::mat4 get_matrix() const;

    const GenericShader& shader;

    glm::vec3 position{0, 0, 0};
    glm::vec3 scale{1, 1, 1};

    VAO vao;
    StaticVBO geometry;
    StaticVBO colors;
    StaticIBO ibo;
    GLuint size;

    GLuint DRAW_MODE{GL_LINES};
};