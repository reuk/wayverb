#pragma once

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/vao.h"

#include <numeric>

class mesh_shader final {
public:
    mesh_shader();

    auto get_attrib_location_v_position() const {
        return program_.get_attrib_location("v_position");
    }

    auto get_attrib_location_v_colour() const {
        return program_.get_attrib_location("v_colour");
    }

    void set_model_matrix(const glm::mat4& mat) const;
    void set_view_matrix(const glm::mat4& mat) const;
    void set_projection_matrix(const glm::mat4& mat) const;

    auto get_scoped() const { return program_.get_scoped(); }

private:
    static const char* vertex_shader;
    static const char* fragment_shader;

    mglu::program program_;
};

////////////////////////////////////////////////////////////////////////////////

class mesh_object final : public mglu::drawable {
public:
    mesh_object(const std::shared_ptr<mesh_shader>& shader,
                const glm::vec3* positions,
                size_t num_positions);

    void set_colours(const glm::vec4* colours, size_t num_colours);

private:
    void do_draw(const glm::mat4& matrix) const override;
    glm::mat4 get_local_model_matrix() const override;

    std::weak_ptr<mesh_shader> shader_;

    mglu::vao vao_;
    mglu::static_vbo geometry_;
    mglu::dynamic_vbo colours_;
    mglu::static_ibo ibo;
};
