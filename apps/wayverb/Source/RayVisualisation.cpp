#include "RayVisualisation.hpp"

#include "OtherComponents/MoreConversions.hpp"

#include "common/cl_traits.h"

namespace {

aligned::vector<glm::vec3> extract_positions(
        const aligned::vector<aligned::vector<Impulse>>& impulses,
        const glm::vec3& source) {
    aligned::vector<glm::vec3> ret;
    ret.reserve(impulses.size() * impulses.front().size() + 1);

    //  first in the buffer is the source position
    ret.push_back(source);

    //  now add positions ray-by-ray from the input
    for (const auto& ray : impulses) {
        for (const auto& reflection : ray) {
            ret.push_back(to_glm_vec3(reflection.position));
        }
    }

    return ret;
}

aligned::vector<float> extract_pressures(
        const aligned::vector<aligned::vector<Impulse>>& impulses) {
    aligned::vector<float> ret;
    ret.reserve(impulses.size() * impulses.front().size() + 1);

    //  source
    ret.push_back(1);

    //  impulses
    for (const auto& ray : impulses) {
        for (const auto& reflection : ray) {
            ret.push_back(std::abs(mean(reflection.volume)));
        }
    }

    return ret;
}

aligned::vector<GLuint> compute_indices(
        const aligned::vector<aligned::vector<Impulse>>& impulses) {
    aligned::vector<GLuint> ret;

    //  impulses
    auto counter = 1u;
    for (const auto& ray : impulses) {
        if (!ray.empty()) {
            ret.push_back(0);  //  source

            const auto lim = counter + ray.size() - 1;
            for (; counter != lim; ++counter) {
                for (auto i = 0; i != 2; ++i) {
                    ret.push_back(counter);
                }
            }

            ret.push_back(counter);
            counter += 1;
        }
    }

    return ret;
}

}  // namespace

RayVisualisation::RayVisualisation(
        const RayShader& shader,
        const aligned::vector<aligned::vector<Impulse>>& impulses,
        const glm::vec3& source)
        : shader(shader)
        , positions(extract_positions(impulses, source))
        , pressures(extract_pressures(impulses))
        , ibo(compute_indices(impulses))
        , source(source) {
    const auto s_vao = get_scoped(vao);

    const auto bind_buffer = [this](
            auto& buffer, auto pos, GLint size, GLenum type) {
        buffer.bind();
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, size, type, GL_FALSE, 0, nullptr);
    };

    bind_buffer(
            positions, shader.get_attrib_location_v_position(), 3, GL_FLOAT);
    bind_buffer(
            pressures, shader.get_attrib_location_v_pressure(), 1, GL_FLOAT);
    ibo.bind();
}

void RayVisualisation::do_draw(const glm::mat4& modelview_matrix) const {
    auto s_shader = mglu::get_scoped(shader);
    shader.set_model_matrix(modelview_matrix);

    auto s_vao = get_scoped(vao);
    glDrawElements(GL_LINES, ibo.size(), GL_UNSIGNED_INT, nullptr);
}

glm::mat4 RayVisualisation::get_local_modelview_matrix() const {
    return glm::mat4{};
}