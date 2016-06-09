#pragma once

#include "BasicDrawableObject.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

class TexturedQuadShader : public ShaderProgram {
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

class AxisObject final : public BasicDrawableObject {
public:
    AxisObject(ShaderProgram& shader, TexturedQuadShader& quad_shader);

    void set_label(const std::string& t);

    void draw() const override;

private:
    TexturedQuadShader* quad_shader;
    OpenGLTexture texture;
};
