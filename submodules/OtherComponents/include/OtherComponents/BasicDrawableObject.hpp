#pragma once

#include "common/aligned/vector.h"
#include "common/orientable.h"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/fbo.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/render_buffer.h"
#include "modern_gl_utils/screen_quad.h"
#include "modern_gl_utils/texture_object.h"
#include "modern_gl_utils/updatable.h"
#include "modern_gl_utils/vao.h"

class Node : public Orientable {
public:
    Node() = default;

    Node(Node&&) noexcept;
    Node& operator=(Node&&) noexcept;

    glm::vec3 get_position() const;
    void set_position(const glm::vec3& v);

    glm::vec3 get_scale() const;
    void set_scale(float s);
    void set_scale(const glm::vec3& s);

    glm::mat4 get_matrix() const;

private:
    glm::vec3 position{0};
    glm::vec3 scale{1};
};

class BasicDrawableObject : public mglu::drawable, public Node {
public:
    template <typename T>
    BasicDrawableObject(T& shader,
                        const aligned::vector<glm::vec3>& g,
                        const aligned::vector<glm::vec4>& c,
                        const aligned::vector<GLuint>& i,
                        GLuint mode)
            : shader(std::make_unique<shader_temp<T>>(shader))
            , color_vector(c)
            , mode(mode) {
        geometry.data(g);
        set_highlight(0);
        ibo.data(i);

        auto s_vao = get_scoped(vao);

        geometry.bind();
        auto v_pos = this->shader->get_attrib_location_v_position();
        glEnableVertexAttribArray(v_pos);
        glVertexAttribPointer(v_pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        colors.bind();
        auto c_pos = this->shader->get_attrib_location_v_color();
        glEnableVertexAttribArray(c_pos);
        glVertexAttribPointer(c_pos, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

        ibo.bind();
    }

    BasicDrawableObject(BasicDrawableObject&&) noexcept;
    BasicDrawableObject& operator=(BasicDrawableObject&&) noexcept;

    GLuint get_mode() const;
    void set_mode(GLuint mode);

    void set_highlight(float amount);

private:
    class shader_base {
    public:
        virtual ~shader_base() noexcept                       = default;
        virtual GLuint get_attrib_location_v_position() const = 0;
        virtual GLuint get_attrib_location_v_color() const    = 0;
        virtual void set_model_matrix(const glm::mat4&) const = 0;
        virtual mglu::usable::scoped get_scoped() const       = 0;
    };

    template <typename T>
    class shader_temp final : public shader_base {
    public:
        shader_temp(T& t)
                : t(&t) {}
        GLuint get_attrib_location_v_position() const override {
            return t->get_attrib_location_v_position();
        }
        GLuint get_attrib_location_v_color() const override {
            return t->get_attrib_location_v_color();
        }
        void set_model_matrix(const glm::mat4& m) const override {
            t->set_model_matrix(m);
        }
        mglu::usable::scoped get_scoped() const override {
            return mglu::get_scoped(*t);
        }

    private:
        T* t;
    };

    void do_draw(const glm::mat4& modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    std::unique_ptr<shader_base> shader;

    aligned::vector<glm::vec4> color_vector;

    mglu::vao vao;
    mglu::static_vbo geometry;
    mglu::static_vbo colors;
    mglu::static_ibo ibo;

    GLuint mode{GL_LINES};
};
