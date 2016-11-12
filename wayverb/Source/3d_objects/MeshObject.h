#pragma once

#include "MeshShader.h"

#include "waveguide/mesh.h"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/vao.h"

class MeshObject final : public mglu::drawable {
public:
    MeshObject(const std::shared_ptr<MeshShader>& shader,
               const util::aligned::vector<glm::vec3>& positions);

    void set_pressures(const util::aligned::vector<float>& u);
    void zero_pressures();

private:
    void do_draw(const glm::mat4& model_matrix) const override;
    glm::mat4 get_local_model_matrix() const override;

    void set_positions(const util::aligned::vector<glm::vec3>& positions);

    std::shared_ptr<MeshShader> shader;

    mglu::vao vao;
    mglu::static_vbo geometry;
    mglu::dynamic_vbo pressures;
    mglu::static_ibo ibo;
};

//----------------------------------------------------------------------------//

class DebugMeshObject final : public mglu::drawable {
public:
    enum class mode { closest_surface, boundary_type };

    DebugMeshObject(const std::shared_ptr<mglu::generic_shader>& shader,
                    const wayverb::waveguide::mesh& model,
                    mode m = mode::boundary_type);

private:
    void do_draw(const glm::mat4& model_matrix) const override;
    glm::mat4 get_local_model_matrix() const override;

    std::shared_ptr<mglu::generic_shader> shader;

    mglu::vao vao;
    mglu::static_vbo geometry;
    mglu::static_vbo colours;
    mglu::static_ibo ibo;
};
