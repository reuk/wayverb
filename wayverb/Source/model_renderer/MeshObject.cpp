#include "MeshObject.hpp"

MeshObject::MeshObject(const MeshShader& shader,
                       const std::vector<glm::vec3>& positions)
        : shader(shader) {
    set_positions(positions);

    //  init vao
    auto s_vao = vao.get_scoped();

    {
        geometry.bind();
        auto pos = shader.get_attrib_location("v_position");
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    {
        pressures.bind();
        auto pos = shader.get_attrib_location("v_pressure");
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    ibo.bind();
}

void MeshObject::draw() const {
    auto s_shader = shader.get_scoped();

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_POINTS, size, GL_UNSIGNED_INT, nullptr);
}

void MeshObject::set_pressures(const std::vector<float>& u) {
    pressures.data(u);
}

void MeshObject::zero_pressures() {
    set_pressures(std::vector<float>(size, 0));
}

void MeshObject::set_positions(const std::vector<glm::vec3>& positions) {
    size = positions.size();

    geometry.data(positions);
    zero_pressures();

    std::vector<GLuint> indices(positions.size());
    std::iota(indices.begin(), indices.end(), 0);
    ibo.data(indices);
}