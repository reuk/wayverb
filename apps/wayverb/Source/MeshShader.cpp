#include "MeshShader.hpp"

MeshShader::MeshShader()
        : program(mglu::program::from_sources(vertex_shader, fragment_shader)) {
}

void MeshShader::set_model_matrix(const glm::mat4& mat) const {
    program.set("v_model", mat);
}
void MeshShader::set_view_matrix(const glm::mat4& mat) const {
    program.set("v_view", mat);
}
void MeshShader::set_projection_matrix(const glm::mat4& mat) const {
    program.set("v_projection", mat);
}

const char* MeshShader::vertex_shader(R"(
#version 150
in vec3 v_position;
in float v_pressure;
out vec4 f_color;
uniform mat4 v_model;
uniform mat4 v_view;
uniform mat4 v_projection;

const float min_point = 0.1;
const float max_point = 8.0;
const float min_color = 0.9;
const float max_color = 1.0;
const float max_dist = 20.0;

const float amp = 1000.0;

void main() {
    vec4 cs_position = v_view * v_model * vec4(v_position, 1.0);
    gl_Position = v_projection * cs_position;

    float dist = -cs_position.z;
    float scaled = 1.0 - (dist / max_dist);
    float p = clamp(scaled * (max_point - min_point) + min_point, min_point, max_point);
    float c = clamp(scaled * (max_color - min_color) + min_color, min_color, max_color);
    gl_PointSize = p;
    float m = v_pressure * amp;
    f_color = mix(vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 1.0, 1.0), clamp(m + 0.5, 0.0, 1.0)) * abs(m);
}
)");

const char* MeshShader::fragment_shader(R"(
#version 150
in vec4 f_color;
out vec4 frag_color;
void main() {
    frag_color = f_color;
}
)");
