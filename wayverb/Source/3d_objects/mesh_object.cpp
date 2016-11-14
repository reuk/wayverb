#include "mesh_object.h"

#include "waveguide/waveguide.h"

#include "utilities/popcount.h"

#include "glm/gtc/random.hpp"

mesh_shader::mesh_shader()
        : program_{mglu::program::from_sources(vertex_shader,
                                               fragment_shader)} {}

void mesh_shader::set_model_matrix(const glm::mat4& mat) const {
    program_.set("v_model", mat);
}
void mesh_shader::set_view_matrix(const glm::mat4& mat) const {
    program_.set("v_view", mat);
}
void mesh_shader::set_projection_matrix(const glm::mat4& mat) const {
    program_.set("v_projection", mat);
}

const char* mesh_shader::vertex_shader = R"(
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

void main() {
    vec4 cs_position = v_view * v_model * vec4(v_position, 1.0);
    gl_Position = v_projection * cs_position;

    float dist = -cs_position.z;
    float scaled = 1.0 - (dist / max_dist);
    float p = clamp(scaled * (max_point - min_point) + min_point, min_point, max_point);
    float c = clamp(scaled * (max_color - min_color) + min_color, min_color, max_color);
    gl_PointSize = p;

    float a = abs(v_pressure) * 1000.0;
    f_color = 0.0 < v_pressure ? vec4(a, 0.0, 0.0, a) : vec4(0.0, a, a, a);
}
)";

const char* mesh_shader::fragment_shader = R"(
#version 150
in vec4 f_color;
out vec4 frag_color;
void main() {
    frag_color = f_color;
}
)";

////////////////////////////////////////////////////////////////////////////////

mesh_object::mesh_object(const std::shared_ptr<mesh_shader>& shader,
                         const glm::vec3* positions,
                         size_t num_positions)
        : shader_{shader} {
    std::vector<GLuint> indices(num_positions);
    std::iota(indices.begin(), indices.end(), 0);
    ibo.data(indices);

    geometry_.data(positions, num_positions);

    {
        std::vector<float> colours(num_positions, 0.0f);
        set_pressures(colours.data(), colours.size());
    }

    //  init vao
    const auto s_vao = vao_.get_scoped();
    mglu::enable_and_bind_buffer(vao_,
                                 geometry_,
                                 shader->get_attrib_location_v_position(),
                                 3,
                                 GL_FLOAT);

    mglu::enable_and_bind_buffer(vao_,
                                 pressures_,
                                 shader->get_attrib_location_v_pressure(),
                                 1,
                                 GL_FLOAT);
    ibo.bind();
}

void mesh_object::set_pressures(const float* pressures, size_t num_pressures) {
    pressures_.data(pressures, num_pressures);
}

void mesh_object::do_draw(const glm::mat4& matrix) const {
    if (const auto shader = shader_.lock()) {
        glDepthMask(GL_FALSE);
        auto s_shader = shader->get_scoped();
        shader->set_model_matrix(matrix);

        auto s_vao = vao_.get_scoped();
        glDrawElements(GL_POINTS, ibo.size(), GL_UNSIGNED_INT, nullptr);
        glDepthMask(GL_TRUE);
    }
}

glm::mat4 mesh_object::get_local_model_matrix() const { return glm::mat4{}; }
