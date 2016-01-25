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

//----------------------------------------------------------------------------//

template <typename WaveguideType>
class MeshObject final : public ::Drawable {
public:
    MeshObject(const GenericShader & shader, const WaveguideType & waveguide)
            : shader(shader)
            , node_type(extract_node_type(waveguide.get_mesh().get_nodes())) {
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
        std::transform(node_type.begin(),
                       node_type.end(),
                       c.begin(),
                       [](const auto & i) {
                           auto c = i == id_inside ? 1 : 0;
                           return glm::vec4(c, c, c, c);
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
                       node_type.begin(),
                       c.begin(),
                       [this](auto i, auto j) {
#define SHOW_BOUNDARIES 0
#if SHOW_BOUNDARIES
                           if (j == id_boundary) {
                               return glm::vec4(1, 1, 1, 1);
                           } else {
#endif
                               auto p = i * amp;
                               return p > 0 ? glm::vec4(0, p, p, p)
                                            : glm::vec4(-p, 0, 0, -p);
#if SHOW_BOUNDARIES
                           }
#endif
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

    std::vector<NodeType> node_type;

    float amp{100};
};
