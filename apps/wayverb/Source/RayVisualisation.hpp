#pragma once

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/vao.h"

#include "raytracer/cl_structs.h"
#include "raytracer/results.h"

#include "common/aligned/vector.h"

class RayShader {
public:
    RayShader();

    auto get_attrib_location_v_position() const {
        return program.get_attrib_location("v_position");
    }
    auto get_attrib_location_v_pressure() const {
        return program.get_attrib_location("v_pressure");
    }

    auto get_scoped() const { return program.get_scoped(); }

    void set_model_matrix(const glm::mat4& m) const;

private:
    static const char* vert;
    static const char* frag;

    mglu::program program;
};

class RayVisualisation final : public mglu::drawable {
public:
    RayVisualisation(const RayShader& shader,
                     const aligned::vector<aligned::vector<Impulse>>& impulses,
                     const glm::vec3& source,
                     const glm::vec3& receiver);

    RayVisualisation(const RayShader& shader,
                     const raytracer::results& results,
                     size_t rays,
                     const glm::vec3& source);

    void set_time(float t);

private:
    void do_draw(const glm::mat4& modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    const RayShader& shader;
    mglu::vao vao;
    mglu::static_vbo positions;
    mglu::static_vbo pressures;
    mglu::static_ibo ibo;
    glm::vec3 source;
    glm::vec3 receiver;
};