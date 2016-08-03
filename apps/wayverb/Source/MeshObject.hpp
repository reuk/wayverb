#pragma once

#include "MeshShader.hpp"

#include "common/sinc.h"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/vao.h"

class MeshObject final : public mglu::drawable {
public:
    MeshObject(const MeshShader& shader,
               const aligned::vector<glm::vec3>& positions);

    void set_pressures(const aligned::vector<float>& u);
    void zero_pressures();

private:
    void do_draw(const glm::mat4& modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    void set_positions(const aligned::vector<glm::vec3>& positions);

    const MeshShader& shader;

    mglu::vao vao;
    mglu::static_vbo geometry;
    mglu::dynamic_vbo pressures;
    mglu::static_ibo ibo;
    GLuint size{0};
};
