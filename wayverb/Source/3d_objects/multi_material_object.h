#pragma once

#include "LitSceneShader.h"

#include "core/gpu_scene_data.h"

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
            const wayverb::core::gpu_scene_data &scene_data);

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
        single_material_section(const wayverb::core::gpu_scene_data &scene_data,
                                int material_index);

    private:
        void do_draw(const glm::mat4 &model_matrix) const override;
        glm::mat4 get_local_model_matrix() const override;

        static util::aligned::vector<GLuint> get_indices(
                const wayverb::core::gpu_scene_data &scene_data,
                size_t material_index);

        mglu::static_ibo ibo;
        GLuint size;
    };

    util::aligned::vector<single_material_section> sections_;
};
