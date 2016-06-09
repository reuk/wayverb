#include "FadeShader.hpp"

FadeShader::FadeShader()
        : ShaderProgram(vertex_shader, fragment_shader) {
}

void FadeShader::set_fade(float f) const{
    set("v_fade", f);
}

void FadeShader::set_model_matrix(const glm::mat4& mat) const {
    set("v_model", mat);
}
void FadeShader::set_view_matrix(const glm::mat4& mat) const {
    set("v_view", mat);
}
void FadeShader::set_projection_matrix(const glm::mat4& mat) const {
    set("v_projection", mat);
}

const std::string FadeShader::vertex_shader(R"(
#version 150
in vec3 v_position;
in vec4 v_color;
out vec4 f_color;
uniform mat4 v_model;
uniform mat4 v_view;
uniform mat4 v_projection;
uniform float v_fade;

void main() {
    gl_Position = v_projection * v_view * v_model * vec4(v_position, 1.0);
    f_color = v_color * v_fade;;
}
)");

const std::string FadeShader::fragment_shader(R"(
#version 150
in vec4 f_color;
out vec4 frag_color;
void main() {
    frag_color = f_color;
}
)");