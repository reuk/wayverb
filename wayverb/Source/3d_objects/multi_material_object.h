#pragma once

#include "LitSceneShader.h"

#include "core/conversions.h"
#include "core/scene_data.h"

#include "utilities/map_to_vector.h"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/vao.h"

#include <experimental/optional>
#include <memory>

class multi_material_object : public mglu::drawable {
public:
    multi_material_object(
            const std::shared_ptr<mglu::generic_shader> &generic_shader,
            const std::shared_ptr<LitSceneShader> &lit_scene_shader,
            const wayverb::core::triangle *triangles,
            size_t num_triangles,
            const glm::vec3 *vertices,
            size_t num_vertices);

    void set_highlighted(std::experimental::optional<size_t> highlighted);

private:
    void do_draw(const glm::mat4 &model_matrix) const override;
    glm::mat4 get_local_model_matrix() const override;

    std::weak_ptr<mglu::generic_shader> generic_shader_;
    std::weak_ptr<LitSceneShader> lit_scene_shader_;

    mglu::vao wire_vao_;
    mglu::vao fill_vao_;
    mglu::static_vbo geometry_;
    mglu::static_vbo colors_;

    std::experimental::optional<size_t> highlighted_ =
            std::experimental::nullopt;

    class single_material_section : public mglu::drawable {
    public:
        single_material_section(const wayverb::core::triangle *triangles,
                                size_t num_triangles,
                                size_t material_index);

        size_t get_material_index() const;

    private:
        void do_draw(const glm::mat4 &model_matrix) const override;
        glm::mat4 get_local_model_matrix() const override;

        static util::aligned::vector<GLuint> get_indices(
                const wayverb::core::triangle *triangles,
                size_t num_triangles,
                size_t material_index);

        size_t material_index_;
        mglu::static_ibo ibo_;
    };

    util::aligned::vector<single_material_section> sections_;
};
