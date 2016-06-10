#pragma once

#include "glm/glm.hpp"
#include "modern_gl_utils/shader_program.h"

class WaterfallShader final : public ShaderProgram {
public:
    WaterfallShader();

    void set_fade(float f) const;

    void set_model_matrix(const glm::mat4& mat) const;
    void set_view_matrix(const glm::mat4& mat) const;
    void set_projection_matrix(const glm::mat4& mat) const;

private:
    static const std::string vert;
    static const std::string frag;
};
