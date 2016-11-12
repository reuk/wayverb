#include "BasicDrawableObject.h"

#include "utilities/map_to_vector.h"

#include <vector>

glm::vec3 node::get_position() const { return position_; }
void node::set_position(const glm::vec3& v) { position_ = v; }

glm::vec3 node::get_scale() const { return scale_; }
void node::set_scale(float s) { scale_ = glm::vec3{s}; }
void node::set_scale(const glm::vec3& s) { scale_ = s; }

glm::vec3 node::get_pointing() const { return orientable_.get_pointing(); }
void node::set_pointing(const glm::vec3& u) { orientable_.set_pointing(u); }

glm::mat4 node::get_matrix() const {
    return glm::translate(get_position()) * orientable_.get_matrix() *
           glm::scale(get_scale());
}

//----------------------------------------------------------------------------//

BasicDrawableObject::BasicDrawableObject(BasicDrawableObject&&) noexcept =
        default;
BasicDrawableObject& BasicDrawableObject::operator=(
        BasicDrawableObject&&) noexcept           = default;

void BasicDrawableObject::set_highlight(float amount) {
    colors.data(util::map_to_vector(begin(color_vector),
                                    end(color_vector),
                                    [&](const auto& i) { return i + amount; }));
}

void BasicDrawableObject::do_draw(const glm::mat4& model_matrix) const {
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(model_matrix);

    auto s_vao = vao.get_scoped();
    glDrawElements(mode, ibo.size(), GL_UNSIGNED_INT, nullptr);
}

glm::mat4 BasicDrawableObject::get_local_model_matrix() const {
    return get_matrix();
}

GLuint BasicDrawableObject::get_mode() const { return mode; }
void BasicDrawableObject::set_mode(GLuint u) { mode = u; }
