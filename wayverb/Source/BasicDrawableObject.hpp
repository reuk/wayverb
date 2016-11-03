#pragma once

#include "orientable.h"

#include "utilities/aligned/vector.h"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/fbo.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/render_buffer.h"
#include "modern_gl_utils/screen_quad.h"
#include "modern_gl_utils/texture_object.h"
#include "modern_gl_utils/updatable.h"
#include "modern_gl_utils/vao.h"

class node {
public:
    node() = default;

    node(const node&) = default;
    node(node&&) noexcept = default;

    node& operator=(const node&) = default;
    node& operator=(node&&) noexcept = default;

    virtual ~node() noexcept = default;

    glm::vec3 get_position() const;
    void set_position(const glm::vec3& v);

    glm::vec3 get_scale() const;
    void set_scale(float s);
    void set_scale(const glm::vec3& s);

    glm::vec3 get_pointing() const;
    void set_pointing(const glm::vec3& u);

    glm::mat4 get_matrix() const;

private:
    glm::vec3 position_{0};
    glm::vec3 scale_{1};
    orientable orientable_;
};

class BasicDrawableObject : public mglu::drawable, public node {
public:
    template <typename T>
    BasicDrawableObject(const std::shared_ptr<T>& shader,
                        const util::aligned::vector<glm::vec3>& g,
                        const util::aligned::vector<glm::vec4>& c,
                        const util::aligned::vector<GLuint>& i,
                        GLuint mode)
            : shader(std::make_unique<shader_temp<T>>(shader))
            , color_vector(c)
            , geometry(g)
            , ibo(i)
            , mode(mode) {
        set_highlight(0);

        auto s_vao = vao.get_scoped();
        mglu::enable_and_bind_buffer(vao,
                                     geometry,
                                     shader->get_attrib_location_v_position(),
                                     3,
                                     GL_FLOAT);
        mglu::enable_and_bind_buffer(vao,
                                     colors,
                                     shader->get_attrib_location_v_color(),
                                     4,
                                     GL_FLOAT);
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
        virtual ~shader_base() noexcept = default;

        using scoped = decltype(std::declval<mglu::usable>().get_scoped());

        virtual void set_model_matrix(const glm::mat4&) const = 0;
        virtual scoped get_scoped() const = 0;
    };

    template <typename T>
    class shader_temp final : public shader_base {
    public:
        shader_temp(const std::shared_ptr<T>& t)
                : t(t) {}
        void set_model_matrix(const glm::mat4& m) const override {
            t->set_model_matrix(m);
        }
        scoped get_scoped() const override { return t->get_scoped(); }

    private:
        std::shared_ptr<T> t;
    };

    void do_draw(const glm::mat4& modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    std::unique_ptr<shader_base> shader;

    util::aligned::vector<glm::vec4> color_vector;

    mglu::vao vao;
    mglu::static_vbo geometry;
    mglu::static_vbo colors;
    mglu::static_ibo ibo;

    GLuint mode{GL_LINES};
};
