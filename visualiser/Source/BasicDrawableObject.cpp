#include "BasicDrawableObject.hpp"

BasicDrawableObject::BasicDrawableObject(const GenericShader& shader,
                                         const std::vector<glm::vec3>& g,
                                         const std::vector<glm::vec4>& c,
                                         const std::vector<GLuint>& i,
                                         GLuint mode)
        : shader(shader)
        , size(i.size())
        , mode(mode) {
    geometry.data(g);
    colors.data(c);
    ibo.data(i);

    auto s_vao = vao.get_scoped();

    geometry.bind();
    auto v_pos = shader.get_attrib_location("v_position");
    glEnableVertexAttribArray(v_pos);
    glVertexAttribPointer(v_pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    colors.bind();
    auto c_pos = shader.get_attrib_location("v_color");
    glEnableVertexAttribArray(c_pos);
    glVertexAttribPointer(c_pos, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    ibo.bind();
}

void BasicDrawableObject::draw() const {
    auto s_shader = shader.get_scoped();
    shader.set_model_matrix(get_matrix());
    shader.set_black(false);

    auto s_vao = vao.get_scoped();
    glDrawElements(mode, size, GL_UNSIGNED_INT, nullptr);
}

GLuint BasicDrawableObject::get_mode() const {
    return mode;
}
void BasicDrawableObject::set_mode(GLuint u) {
    mode = u;
}

glm::vec3 BasicDrawableObject::get_position() const {
    return position;
}
void BasicDrawableObject::set_position(const glm::vec3& p) {
    position = p;
}

glm::vec3 BasicDrawableObject::get_scale() const {
    return scale;
}
void BasicDrawableObject::set_scale(const glm::vec3& s) {
    scale = s;
}
void BasicDrawableObject::set_scale(float s) {
    scale = glm::vec3(s, s, s);
}

const GenericShader& BasicDrawableObject::get_shader() const {
    return shader;
}

glm::mat4 BasicDrawableObject::get_matrix() const {
    return glm::translate(position) * glm::scale(scale);
}