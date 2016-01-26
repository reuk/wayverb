#pragma once

#include "waveguide.h"

template <typename T>
std::vector<NodeType> extract_node_type(const std::vector<T> & nodes) {
    std::vector<NodeType> ret(nodes.size());
    std::transform(nodes.begin(),
                   nodes.end(),
                   ret.begin(),
                   [](const auto & i) { return i.inside; });
    return ret;
}

template <typename T>
std::vector<cl_int> extract_bitmask(const std::vector<T> & nodes) {
    std::vector<cl_int> ret(nodes.size());
    std::transform(nodes.begin(),
                   nodes.end(),
                   ret.begin(),
                   [](const auto & i) { return i.bt; });
    return ret;
}

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
                       [](auto i) {
                           auto p = i.position;
                           return glm::vec3(p.x, p.y, p.z);
                       });

        //  init buffers
        std::vector<glm::vec4> c(v.size());

        std::transform(nodes.begin(),
                       nodes.end(),
                       c.begin(),
                       [](const auto & i) {
                           switch (i.bt) {
                               case id_none:
                                   return glm::vec4(0, 0, 0, 0);
                               case id_nx:
                                   return glm::vec4(1, 0, 0, 1);
                               case id_px:
                                   return glm::vec4(0, 1, 1, 1);
                               case id_ny:
                                   return glm::vec4(0, 1, 0, 1);
                               case id_py:
                                   return glm::vec4(1, 0, 1, 1);
                               case id_nz:
                                   return glm::vec4(0, 0, 1, 1);
                               case id_pz:
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
        std::vector<glm::vec4> c(pressures.size(), glm::vec4(0, 0, 0, 0));
        std::transform(pressures.begin(),
                       pressures.end(),
                       c.begin(),
                       [this](auto i) {
                           auto p = i * amp;
                           return p > 0 ? glm::vec4(0, p, p, p)
                                        : glm::vec4(-p, 0, 0, -p);
                       });
        colors.data(c);
    }

private:
    const GenericShader & shader;

    VAO vao;
    StaticVBO geometry;
    DynamicVBO colors;
    StaticIBO ibo;
    GLuint size;

    float amp{100};
};
