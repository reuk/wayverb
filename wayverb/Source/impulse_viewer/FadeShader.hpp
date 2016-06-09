#pragma once

#include "glm/glm.hpp"
#include "modern_gl_utils/shader_program.h"

class FadeShader final : public ShaderProgram {
public:
    FadeShader();

    void set_fade(float f);

    void set_model_matrix(const glm::mat4& mat);
    void set_view_matrix(const glm::mat4& mat);
    void set_projection_matrix(const glm::mat4& mat);

private:
    static const std::string vertex_shader;
    static const std::string fragment_shader;
};
