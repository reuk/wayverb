#include "MeshObject.hpp"

MeshObject::MeshObject(const MeshShader& shader,
                       const aligned::vector<glm::vec3>& positions)
        : shader(shader) {
    set_positions(positions);

    //  init vao
    const auto s_vao = vao.get_scoped();

    const auto bind_buffer = [this](
            auto& buffer, auto pos, GLint size, GLenum type) {
        buffer.bind();
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, size, type, GL_FALSE, 0, nullptr);
    };

    bind_buffer(geometry, shader.get_attrib_location_v_position(), 3, GL_FLOAT);
    bind_buffer(
            pressures, shader.get_attrib_location_v_pressure(), 1, GL_FLOAT);

    ibo.bind();
}

void MeshObject::do_draw(const glm::mat4& modelview_matrix) const {
    auto s_shader = shader.get_scoped();
    shader.set_model_matrix(modelview_matrix);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_POINTS, size, GL_UNSIGNED_INT, nullptr);
}

glm::mat4 MeshObject::get_local_modelview_matrix() const { return glm::mat4{}; }

void MeshObject::set_pressures(const aligned::vector<float>& u) {
    pressures.data(u);
}

void MeshObject::zero_pressures() {
    set_pressures(aligned::vector<float>(size, 0));
}

void MeshObject::set_positions(const aligned::vector<glm::vec3>& positions) {
    size = positions.size();

    geometry.data(positions);
    zero_pressures();

    aligned::vector<GLuint> indices(positions.size());
    std::iota(indices.begin(), indices.end(), 0);
    ibo.data(indices);
}