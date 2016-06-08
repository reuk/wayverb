#include "BasicDrawableObject.hpp"

#include "common/stl_wrappers.h"

//Node::Node(Node&& rhs) noexcept : position(rhs.position), scale(rhs.scale) {
//}
//Node& Node::operator=(Node&& rhs) noexcept {
//    std::tie(position, scale) = std::tie(rhs.position, rhs.scale);
//    return *this;
//}

Node::Node(Node&&) noexcept = default;
Node& Node::operator=(Node&&) noexcept = default;

glm::vec3 Node::get_position() const {
    return position;
}

void Node::set_position(const glm::vec3& v) {
    position = v;
}

float Node::get_scale() const {
    return scale;
}

void Node::set_scale(float s) {
    scale = s;
}

glm::mat4 Node::get_matrix() const {
    return glm::translate(get_position()) * Orientable::get_matrix() *
           glm::scale(glm::vec3{get_scale()});
}

//----------------------------------------------------------------------------//

BasicDrawableObject::BasicDrawableObject(GenericShader& shader,
                                         const std::vector<glm::vec3>& g,
                                         const std::vector<glm::vec4>& c,
                                         const std::vector<GLuint>& i,
                                         GLuint mode)
        : shader(&shader)
        , color_vector(c)
        , mode(mode) {
    geometry.data(g);
    set_highlight(0);
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

BasicDrawableObject::BasicDrawableObject(BasicDrawableObject&&) noexcept =
    default;
BasicDrawableObject& BasicDrawableObject::operator=(
    BasicDrawableObject&&) noexcept = default;

void BasicDrawableObject::set_highlight(float amount) {
    std::vector<glm::vec4> highlighted(color_vector.size());
    proc::transform(color_vector,
                    highlighted.begin(),
                    [amount](const auto& i) { return i + amount; });
    colors.data(highlighted);
}

void BasicDrawableObject::draw() const {
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(get_matrix());

    auto s_vao = vao.get_scoped();
    glDrawElements(mode, ibo.size(), GL_UNSIGNED_INT, nullptr);
}

GLuint BasicDrawableObject::get_mode() const {
    return mode;
}
void BasicDrawableObject::set_mode(GLuint u) {
    mode = u;
}

const GenericShader& BasicDrawableObject::get_shader() const {
    return *shader;
}