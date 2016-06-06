#pragma once

#include "MeshShader.hpp"

#include "common/sinc.h"
#include "waveguide/waveguide.h"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/vao.h"

class MeshObject final : public ::Drawable {
public:
    MeshObject(const MeshShader& shader,
               const std::vector<glm::vec3>& positions);

    void draw() const override;

    void set_pressures(const std::vector<float>& u);
    void zero_pressures();

private:
    void set_positions(const std::vector<glm::vec3>& positions);

    const MeshShader& shader;

    VAO vao;
    StaticVBO geometry;
    DynamicVBO pressures;
    StaticIBO ibo;
    GLuint size{0};
};
