#include "WaterfallShader.hpp"

WaterfallShader::WaterfallShader()
        : ShaderProgram(vert, frag) {
}

void WaterfallShader::set_fade(float f) const {
    set("v_fade", f);
}

void WaterfallShader::set_model_matrix(const glm::mat4& mat) const {
    set("v_model", mat);
}
void WaterfallShader::set_view_matrix(const glm::mat4& mat) const {
    set("v_view", mat);
}
void WaterfallShader::set_projection_matrix(const glm::mat4& mat) const {
    set("v_projection", mat);
}

const std::string WaterfallShader::vert(R"(
#version 150
in vec3 v_position;
out vec4 f_color;
uniform mat4 v_model;
uniform mat4 v_view;
uniform mat4 v_projection;
uniform float v_fade;

uniform float v_gain = 1.0;

const float min = 0.2;
const float max = 1.0;
const vec3 colors[4] = vec3[](vec3(min, min, min),
                              vec3(min, max, min),
                              vec3(min, max, max),
                              vec3(max, max, max));
const float d = 1.0 / 3.0;

vec3 compute_mapped_color(float r) {
    for (int i = 0; i != 3; ++i, r -= d) {
        if (r < d) {
            return mix(colors[i], colors[i + 1], r * 3.0);
        }
    }
    return colors[0];
}

void main() {
    vec4 pos = vec4(v_position.x, v_position.y * v_gain, v_position.z, 1.0);
    gl_Position = v_projection * v_view * v_model * pos;
    f_color = mix(vec4(0.0, 0.0, 0.0, 1.0),
                  vec4(compute_mapped_color(fract(pos.x)), 1.0),
                  pow(pos.y, 0.01)) * v_fade;
}
)");

const std::string WaterfallShader::frag(R"(
#version 150
in vec4 f_color;
out vec4 frag_color;
void main() {
    frag_color = f_color;
}
)");