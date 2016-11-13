#pragma once

#include "raytracer/cl/reflection.h"

#include "utilities/aligned/vector.h"

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/generic_shader.h"
#include "modern_gl_utils/vao.h"

class reflections_shader final {
public:
    reflections_shader();

    auto get_attrib_location_v_position() const {
        return program_.get_attrib_location("v_position");
    }
    auto get_attrib_location_v_pressure() const {
        return program_.get_attrib_location("v_pressure");
    }

    auto get_scoped() const { return program_.get_scoped(); }

    void set_model_matrix(const glm::mat4& m) const;
    void set_view_matrix(const glm::mat4& u) const;
    void set_projection_matrix(const glm::mat4& u) const;

private:
    static const char* vert;
    static const char* frag;

    mglu::program program_;
};

////////////////////////////////////////////////////////////////////////////////

class reflections_object final : public mglu::drawable {
public:
    reflections_object(const std::shared_ptr<reflections_shader>& shader,
                       const util::aligned::vector<util::aligned::vector<
                               wayverb::raytracer::reflection>>& reflections,
                       const glm::vec3& source);

    void set_distance(double t);

private:
    struct path_data final {
        glm::vec3 position;
        double distance;
    };

    static util::aligned::vector<path_data> convert_to_path_data(
            const util::aligned::vector<wayverb::raytracer::reflection>&
                    reflections,
            const glm::vec3& source);

    static util::aligned::vector<util::aligned::vector<path_data>>
    convert_to_path_data(const util::aligned::vector<util::aligned::vector<
                                 wayverb::raytracer::reflection>>& reflections,
                         const glm::vec3& source);

    static util::aligned::vector<glm::vec3> extract_positions(
            const util::aligned::vector<util::aligned::vector<path_data>>&
                    reflections,
            const glm::vec3& source);

    static util::aligned::vector<float> extract_pressures(
            const util::aligned::vector<util::aligned::vector<path_data>>&
                    reflections);

    static util::aligned::vector<GLuint> compute_indices(
            const util::aligned::vector<util::aligned::vector<path_data>>&
                    reflections,
            double distance,
            size_t reflection_points);

    static glm::vec3 ray_wavefront_position(
            const util::aligned::vector<path_data>& path,
            double distance,
            const glm::vec3& source);

    static util::aligned::vector<glm::vec3> ray_wavefront_position(
            const util::aligned::vector<util::aligned::vector<path_data>>&
                    paths,
            double distance,
            const glm::vec3& source);

    reflections_object(
            const std::shared_ptr<reflections_shader>& shader,
            const util::aligned::vector<util::aligned::vector<path_data>>&
                    reflections,
            const glm::vec3& source,
            size_t reflection_points);

    void do_draw(const glm::mat4& model_matrix) const override;
    glm::mat4 get_local_model_matrix() const override;

    std::weak_ptr<reflections_shader> shader_;

    mglu::vao vao_;
    mglu::dynamic_vbo positions_;
    mglu::static_vbo pressures_;
    mglu::dynamic_ibo ibo_;
    glm::vec3 source_;

    util::aligned::vector<util::aligned::vector<path_data>> paths_;
    size_t reflection_points_;
};
