#include "multi_material_object.h"

#include <unordered_set>

multi_material_object::multi_material_object(
        const std::shared_ptr<mglu::generic_shader> &generic_shader,
        const std::shared_ptr<LitSceneShader> &lit_scene_shader,
        const wayverb::core::triangle *triangles,
        size_t num_triangles,
        const glm::vec3 *vertices,
        size_t num_vertices)
        : generic_shader_{generic_shader}
        , lit_scene_shader_{lit_scene_shader} {
    std::unordered_set<size_t> unique_surfaces;
    for (auto i = triangles, e = triangles + num_triangles; i != e; ++i) {
        unique_surfaces.insert(i->surface);
    }

    for (const auto &i : unique_surfaces) {
        sections_.emplace_back(triangles, num_triangles, i);
    }

    geometry_.data(vertices, num_vertices);

    mglu::check_for_gl_error();
    colors_.data(util::aligned::vector<glm::vec4>(
            num_vertices, glm::vec4(0.5, 0.5, 0.5, 1.0)));
    mglu::check_for_gl_error();

    const auto configure_vao = [this](const auto &vao, const auto &shader) {
        const auto s_vao = vao.get_scoped();
        mglu::check_for_gl_error();
        mglu::enable_and_bind_buffer(vao,
                                     geometry_,
                                     shader->get_attrib_location_v_position(),
                                     3,
                                     GL_FLOAT);
        mglu::check_for_gl_error();
        mglu::enable_and_bind_buffer(vao,
                                     colors_,
                                     shader->get_attrib_location_v_color(),
                                     4,
                                     GL_FLOAT);
        mglu::check_for_gl_error();
    };

    configure_vao(wire_vao_, generic_shader);
    configure_vao(fill_vao_, lit_scene_shader);
}

glm::mat4 multi_material_object::get_local_model_matrix() const {
    return glm::mat4{};
}

void multi_material_object::do_draw(const glm::mat4 &model_matrix) const {
    for (auto i = 0u; i != sections_.size(); ++i) {
        if (highlighted_ && i == *highlighted_) {
            if (const auto shader = lit_scene_shader_.lock()) {
                const auto s_shader = shader->get_scoped();
                shader->set_model_matrix(model_matrix);
                const auto s_vao = fill_vao_.get_scoped();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                sections_[i].draw(model_matrix);
            }

        } else {
            if (const auto shader = generic_shader_.lock()) {
                const auto s_shader = shader->get_scoped();
                shader->set_model_matrix(model_matrix);
                const auto s_vao = wire_vao_.get_scoped();
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                sections_[i].draw(model_matrix);
            }
        }
    }
}

void multi_material_object::set_highlighted(
        std::experimental::optional<size_t> highlighted) {
    highlighted_ = highlighted;
}

////////////////////////////////////////////////////////////////////////////////

multi_material_object::single_material_section::single_material_section(
        const wayverb::core::triangle *triangles,
        size_t num_triangles,
        size_t material_index) {
    const auto indices = get_indices(triangles, num_triangles, material_index);
    size = indices.size();
    ibo.data(indices);
}

void multi_material_object::single_material_section::do_draw(
        const glm::mat4 &) const {
    ibo.bind();
    glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr);
}

glm::mat4
multi_material_object::single_material_section::get_local_model_matrix() const {
    return glm::mat4{};
}

util::aligned::vector<GLuint>
multi_material_object::single_material_section::get_indices(
        const wayverb::core::triangle *triangles,
        size_t num_triangles,
        size_t material_index) {
    util::aligned::vector<GLuint> ret;
    for (auto i = triangles, e = triangles + num_triangles; i != e; ++i) {
        if (i->surface == material_index) {
            ret.emplace_back(i->v0);
            ret.emplace_back(i->v1);
            ret.emplace_back(i->v2);
        }
    }
    return ret;
}
