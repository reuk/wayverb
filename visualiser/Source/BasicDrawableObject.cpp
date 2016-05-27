#include "BasicDrawableObject.hpp"

#include <glm/gtx/perpendicular.hpp>

glm::vec3 Node::get_position() const {
    return position;
}
void Node::set_position(const glm::vec3& p) {
    position = p;
    recompute_matrix();
}

glm::vec3 Node::get_scale() const {
    return scale;
}
void Node::set_scale(const glm::vec3& s) {
    scale = s;
    recompute_matrix();
}
void Node::set_scale(float s) {
    scale = glm::vec3(s);
    recompute_matrix();
}

glm::vec3 Node::get_pointing() const {
    return pointing;
}

void Node::set_pointing(const glm::vec3& u) {
    pointing = glm::normalize(u);
    recompute_matrix();
}
void Node::look_at(const glm::vec3& u) {
    set_pointing(u - get_position());
}

float Node::get_azimuth() const {
    return std::atan2(pointing.x, pointing.z);
}
float Node::get_elevation() const {
    return glm::asin(pointing.y);
}
Node::AzEl Node::get_azimuth_elevation() const {
    return AzEl{get_azimuth(), get_elevation()};
}

void Node::set_azimuth(float u) {
    set_azimuth_elevation(AzEl{u, get_elevation()});
}
void Node::set_elevation(float u) {
    set_azimuth_elevation(AzEl{get_azimuth(), u});
}
void Node::set_azimuth_elevation(const AzEl& azel) {
    pointing = glm::vec3(glm::sin(azel.azimuth) * glm::cos(azel.elevation),
                         glm::sin(azel.elevation),
                         glm::cos(azel.azimuth) * glm::cos(azel.elevation));
    recompute_matrix();
}

glm::mat4 Node::get_matrix() const {
    return matrix;
}

glm::mat4 Node::compute_matrix(const glm::vec3& position,
                               const glm::vec3& scale,
                               const glm::vec3& pointing) {
    auto z_axis = pointing;
    auto x_axis = glm::normalize(glm::cross(pointing, glm::vec3(0, -1, 0)));
    auto y_axis = glm::normalize(glm::cross(z_axis, x_axis));
    return glm::translate(position) * glm::scale(scale) *
           glm::mat4(glm::vec4(x_axis, 0),
                     glm::vec4(y_axis, 0),
                     glm::vec4(z_axis, 0),
                     glm::vec4(0, 0, 0, 1));
}

void Node::recompute_matrix() {
    matrix = compute_matrix(position, scale, pointing);
}

//----------------------------------------------------------------------------//

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

    auto s_vao = vao.get_scoped();
    glDrawElements(mode, size, GL_UNSIGNED_INT, nullptr);
}

GLuint BasicDrawableObject::get_mode() const {
    return mode;
}
void BasicDrawableObject::set_mode(GLuint u) {
    mode = u;
}

const GenericShader& BasicDrawableObject::get_shader() const {
    return shader;
}