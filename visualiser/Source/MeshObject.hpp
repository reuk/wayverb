#pragma once

#include "waveguide.h"

//----------------------------------------------------------------------------//

template <typename WaveguideType>
class MeshObject final : public ::Drawable {
public:
    MeshObject(const GenericShader & shader, const WaveguideType & waveguide)
            : shader(shader) {
        const auto & nodes = waveguide.get_mesh().get_nodes();

        std::vector<glm::vec3> v(nodes.size());
        std::transform(nodes.begin(),
                       nodes.end(),
                       v.begin(),
                       [](auto i) { return to_glm_vec3(i.position); });

        //  init buffers
        std::vector<glm::vec4> c(v.size());

        std::transform(nodes.begin(),
                       nodes.end(),
                       c.begin(),
                       [](const auto & i) {
                           switch (i.bt) {
                               case RectangularProgram::id_none:
                               case RectangularProgram::id_inside:
                                   return glm::vec4(0, 0, 0, 0);
                               case RectangularProgram::id_nx:
                                   return glm::vec4(1, 0, 0, 1);
                               case RectangularProgram::id_px:
                                   return glm::vec4(0, 1, 1, 1);
                               case RectangularProgram::id_ny:
                                   return glm::vec4(0, 1, 0, 1);
                               case RectangularProgram::id_py:
                                   return glm::vec4(1, 0, 1, 1);
                               case RectangularProgram::id_nz:
                                   return glm::vec4(0, 0, 1, 1);
                               case RectangularProgram::id_pz:
                                   return glm::vec4(1, 1, 0, 1);

                               default:
                                   return glm::vec4(1, 1, 1, 1);
                           }
                           return glm::vec4(0, 0, 0, 0);
                       });

        std::vector<GLuint> indices(v.size());
        std::iota(indices.begin(), indices.end(), 0);

        size = indices.size();

        geometry.data(v);
        colors.data(c);
        ibo.data(indices);

        //  init vao
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

    void draw() const override {
        auto s_shader = shader.get_scoped();
        shader.set_black(false);

        auto s_vao = vao.get_scoped();
        glDrawElements(GL_POINTS, size, GL_UNSIGNED_INT, nullptr);
    }

    void set_pressures(const std::vector<float> & pressures) {
        color_storage.resize(pressures.size());
        std::transform(pressures.begin(),
                       pressures.end(),
                       color_storage.begin(),
                       [this](auto i) {
                           auto p = i * amp;
                           return p > 0 ? glm::vec4(0, p, p, p)
                                        : glm::vec4(-p, 0, 0, -p);
                       });
        colors.data(color_storage);
    }

private:
    const GenericShader & shader;

    std::vector<glm::vec4>
        color_storage;  //  hopefully we don't have to malloc every frame

    VAO vao;
    StaticVBO geometry;
    DynamicVBO colors;
    StaticIBO ibo;
    GLuint size;

    float amp{100};
};
