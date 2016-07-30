#include "OtherComponents/BasicDrawableObject.hpp"

#include "common/aligned/vector.h"

#include <vector>

Node::Node(Node&&) noexcept = default;
Node& Node::operator=(Node&&) noexcept = default;

glm::vec3 Node::get_position() const { return position; }

void Node::set_position(const glm::vec3& v) { position = v; }

glm::vec3 Node::get_scale() const { return scale; }

void Node::set_scale(float s) { scale = glm::vec3{s}; }

void Node::set_scale(const glm::vec3& s) { scale = s; }

glm::mat4 Node::get_matrix() const {
    return glm::translate(get_position()) * Orientable::get_matrix() *
           glm::scale(get_scale());
}

//----------------------------------------------------------------------------//

BasicDrawableObject::BasicDrawableObject(BasicDrawableObject&&) noexcept =
        default;
BasicDrawableObject& BasicDrawableObject::operator=(
        BasicDrawableObject&&) noexcept           = default;

void BasicDrawableObject::set_highlight(float amount) {
    aligned::vector<glm::vec4> highlighted(color_vector.size());
    std::transform(color_vector.begin(),
                   color_vector.end(),
                   highlighted.begin(),
                   [amount](const auto& i) { return i + amount; });
    colors.data(highlighted);
}

void BasicDrawableObject::do_draw(const glm::mat4& modelview_matrix) const {
    auto s_shader = mglu::get_scoped(*shader);
    shader->set_model_matrix(modelview_matrix);

    auto s_vao = get_scoped(vao);
    glDrawElements(mode, ibo.size(), GL_UNSIGNED_INT, nullptr);
}

glm::mat4 BasicDrawableObject::get_local_modelview_matrix() const {
    return get_matrix();
}

GLuint BasicDrawableObject::get_mode() const { return mode; }
void BasicDrawableObject::set_mode(GLuint u) { mode = u; }
