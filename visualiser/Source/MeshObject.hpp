#pragma once

#include "MeshShader.hpp"

#include "common/sinc.h"
#include "waveguide/waveguide.h"

#include <mutex>

class MeshObject final : public ::Drawable {
public:
    MeshObject(const MeshShader& shader,
               const std::vector<glm::vec3>& positions)
            : shader(shader) {
        set_positions(positions);

        //  init vao
        auto s_vao = vao.get_scoped();

        {
            geometry.bind();
            auto pos = shader.get_attrib_location("v_position");
            glEnableVertexAttribArray(pos);
            glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        }

        {
            pressures.bind();
            auto pos = shader.get_attrib_location("v_pressure");
            glEnableVertexAttribArray(pos);
            glVertexAttribPointer(pos, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
        }

        ibo.bind();
    }

    void draw() const override {
        std::lock_guard<std::mutex> lck(mut);

        auto s_shader = shader.get_scoped();

        auto s_vao = vao.get_scoped();
        glDrawElements(GL_POINTS, size, GL_UNSIGNED_INT, nullptr);
    }

    void set_pressures(const std::vector<float>& u) {
        std::lock_guard<std::mutex> lck(mut);
        set_pressures_internal(u);
    }

private:
    void set_positions(const std::vector<glm::vec3>& positions) {
        size = positions.size();

        geometry.data(positions);

        set_pressures_internal(std::vector<float>(positions.size(), 0));

        std::vector<GLuint> indices(positions.size());
        std::iota(indices.begin(), indices.end(), 0);
        ibo.data(indices);
    }

    void set_pressures_internal(const std::vector<float>& u) {
        pressures.data(u);
    }

    const MeshShader& shader;

    VAO vao;
    StaticVBO geometry;
    DynamicVBO pressures;
    StaticIBO ibo;
    GLuint size{0};

    mutable std::mutex mut;
};
