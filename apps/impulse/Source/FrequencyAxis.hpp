#pragma once

#include "BasicDrawableObject.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

class TexturedQuadShader : public mglu::ShaderProgram {
public:
    TexturedQuadShader();

    void set_billboard(const glm::vec3& m) const;
    void set_billboard_size(const glm::vec2& m) const;
    void set_screen_size(const glm::vec2& m) const;

    void set_model_matrix(const glm::mat4& m) const;
    void set_view_matrix(const glm::mat4& m) const;
    void set_projection_matrix(const glm::mat4& m) const;

    void set_fade(float f) const;

    void set_tex(GLint i) const;

private:
    static const std::string vert;
    static const std::string frag;
};

class TextImage {
public:
    void set_text(const std::string& text, int height);
    const Image& get_image() const;

private:
    Image image;
};

class AxisObject : public mglu::Drawable, public Node {
public:
    AxisObject(mglu::ShaderProgram& shader,
               TexturedQuadShader& quad_shader,
               const std::string& text);

private:
    void do_draw(const glm::mat4& modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    BasicDrawableObject axes;
    TexturedQuadShader* quad_shader;
    OpenGLTexture texture;
};
