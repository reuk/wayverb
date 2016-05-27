#pragma once

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/fbo.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/render_buffer.h"
#include "modern_gl_utils/screen_quad.h"
#include "modern_gl_utils/texture_object.h"
#include "modern_gl_utils/updatable.h"
#include "modern_gl_utils/vao.h"

#include <glm/gtc/quaternion.hpp>

class Node {
public:
    Node() = default;
    Node(const Node&) = default;
    Node& operator=(const Node&) = default;
    Node(Node&&) noexcept = default;
    Node& operator=(Node&&) noexcept = default;
    virtual ~Node() noexcept = default;

    glm::vec3 get_position() const;
    void set_position(const glm::vec3& p);

    glm::vec3 get_scale() const;
    void set_scale(const glm::vec3& s);
    void set_scale(float s);

    glm::vec3 get_pointing() const;
    void set_pointing(const glm::vec3& u);
    void look_at(const glm::vec3& u);

    //  relative to global axes I guess
    struct AzEl {
        float azimuth{0};
        float elevation{0};
    };
    float get_azimuth() const;
    float get_elevation() const;
    AzEl get_azimuth_elevation() const;

    void set_azimuth(float u);
    void set_elevation(float u);
    void set_azimuth_elevation(const AzEl& azel);

    virtual glm::mat4 get_matrix() const;

private:
    static glm::mat4 compute_matrix(const glm::vec3& position,
                                    const glm::vec3& scale,
                                    const glm::vec3& pointing);
    void recompute_matrix();

    glm::vec3 position{0, 0, 0};
    glm::vec3 scale{1, 1, 1};
    glm::vec3 pointing{0, 0, 1};

    glm::mat4 matrix;
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