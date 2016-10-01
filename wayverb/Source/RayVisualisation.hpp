#pragma once

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/vao.h"

#include "raytracer/cl/structs.h"
#include "raytracer/results.h"

#include "common/aligned/vector.h"

class RayShader final {
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
    void set_view_matrix(const glm::mat4& u) const;
    void set_projection_matrix(const glm::mat4& u) const;

private:
    static const char* vert;
    static const char* frag;

    mglu::program program;
};

class RayVisualisation final : public mglu::drawable {
public:
    RayVisualisation(const std::shared_ptr<RayShader>& shader,
                     const aligned::vector<aligned::vector<impulse>>& impulses,
                     const glm::vec3& source,
                     const glm::vec3& receiver);

    void set_distance(double t);

private:
    struct path_data final {
        glm::vec3 position;
        double distance;
        double pressure;
    };

    static aligned::vector<path_data> convert_to_path_data(
            const aligned::vector<impulse>& impulses, const glm::vec3& source);

    static aligned::vector<aligned::vector<path_data>> convert_to_path_data(
            const aligned::vector<aligned::vector<impulse>>& impulses,
            const glm::vec3& source);

    static aligned::vector<glm::vec3> extract_positions(
            const aligned::vector<aligned::vector<path_data>>& impulses,
            const glm::vec3& source);

    static aligned::vector<float> extract_pressures(
            const aligned::vector<aligned::vector<path_data>>& impulses);

    static aligned::vector<GLuint> compute_indices(
            const aligned::vector<aligned::vector<path_data>>& impulses,
            double distance,
            size_t reflection_points);

    static glm::vec3 ray_wavefront_position(
            const aligned::vector<path_data>& path,
            double distance,
            const glm::vec3& source);

    static aligned::vector<glm::vec3> ray_wavefront_position(
            const aligned::vector<aligned::vector<path_data>>& paths,
            double distance,
            const glm::vec3& source);

    RayVisualisation(
            const std::shared_ptr<RayShader>& shader,
            const aligned::vector<aligned::vector<path_data>>& impulses,
            const glm::vec3& source,
            const glm::vec3& receiver,
            size_t reflection_points);

    void do_draw(const glm::mat4& modelview_matrix) const override;
    glm::mat4 get_local_modelview_matrix() const override;

    std::shared_ptr<RayShader> shader;

    mglu::vao vao;
    mglu::dynamic_vbo positions;
    mglu::static_vbo pressures;
    mglu::dynamic_ibo ibo;
    glm::vec3 source;
    glm::vec3 receiver;

    aligned::vector<aligned::vector<path_data>> paths;
    size_t reflection_points;
};