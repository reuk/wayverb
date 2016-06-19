#pragma once

#include "MeshShader.hpp"

#include "common/sinc.h"
#include "waveguide/waveguide.h"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/vao.h"

class MeshObject final : public mglu::Drawable {
public:
    MeshObject(const MeshShader& shader,
               const std::vector<glm::vec3>& positions);

    void set_pressures(const std::vector<float>& u);
    void zero_pressures();

private:
    void do_draw(const glm::mat4& modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    void set_positions(const std::vector<glm::vec3>& positions);

    const MeshShader& shader;

    mglu::VAO vao;
    mglu::StaticVBO geometry;
    mglu::DynamicVBO pressures;
    mglu::StaticIBO ibo;
    GLuint size{0};
};
