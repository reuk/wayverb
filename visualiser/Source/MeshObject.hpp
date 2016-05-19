#pragma once

#include "common/sinc.h"
#include "waveguide/waveguide.h"

#include <mutex>

class MeshObject final : public ::Drawable {
public:
    MeshObject(const GenericShader& shader)
            : shader(shader) {
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
        std::lock_guard<std::mutex> lck(mut);

        auto s_shader = shader.get_scoped();
        shader.set_black(false);

        auto s_vao = vao.get_scoped();
        glDrawElements(GL_POINTS, size, GL_UNSIGNED_INT, nullptr);
    }

    void set_pressures(const std::vector<float>& pressures) {
        std::lock_guard<std::mutex> lck(mut);
        set_pressures_internal(pressures);
    }

    void set_positions(const std::vector<glm::vec3>& positions) {
        std::lock_guard<std::mutex> lck(mut);

        size = positions.size();

        geometry.data(positions);

        set_pressures_internal(std::vector<float>(positions.size(), 1));

        std::vector<GLuint> indices(positions.size());
        std::iota(indices.begin(), indices.end(), 0);
        ibo.data(indices);
    }

private:
    void set_pressures_internal(const std::vector<float>& pressures) {
        color_storage.resize(pressures.size());
        std::cout << "mean: " << mean(pressures) << std::endl;
        std::transform(pressures.begin(),
                       pressures.end(),
                       color_storage.begin(),
                       [this](auto i) {
                           auto p = i * amp;
                           if (std::isnan(p))
                               return glm::vec4(1, 1, 1, 1);
                           return p > 0 ? glm::vec4(0, p, p, p)
                                        : glm::vec4(-p, 0, 0, -p);
                       });
        colors.data(color_storage);
    }

    const GenericShader& shader;

    std::vector<glm::vec4>
        color_storage;  //  hopefully we don't have to malloc every frame

    VAO vao;
    DynamicVBO geometry;
    DynamicVBO colors;
    DynamicIBO ibo;
    GLuint size{0};

    float amp{1000};

    mutable std::mutex mut;
};
