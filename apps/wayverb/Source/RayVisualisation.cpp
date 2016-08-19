#include "RayVisualisation.hpp"

#include "OtherComponents/MoreConversions.hpp"

#include "common/aligned/vector.h"
#include "common/cl_traits.h"
#include "common/config.h"
#include "common/map_to_vector.h"

#include <numeric>

namespace {

class distance_accumulator final {
public:
    distance_accumulator(const glm::vec3& original_position)
            : last_position(original_position) {}

    float operator()(const glm::vec3& i) {
        current_distance += glm::distance(last_position, i);
        last_position = i;
        return current_distance;
    }

private:
    glm::vec3 last_position;
    float current_distance{0};
};

template <typename T>
size_t count(const T& t) {
    return 1;
}
template <typename T>
size_t count(const aligned::vector<T>& t) {
    return std::accumulate(
            t.begin(),
            t.end(),
            size_t{0},
            [](const auto& i, const auto& j) { return i + count(j); });
}

}  // namespace

aligned::vector<RayVisualisation::path_data>
RayVisualisation::convert_to_path_data(const aligned::vector<impulse>& impulses,
                                       const glm::vec3& source) {
    aligned::vector<path_data> ret;
    ret.reserve(impulses.size());

    distance_accumulator d(source);
    for (const auto& i : impulses) {
        const auto pos = to_glm_vec3(i.position);
        const auto dist = d(pos);
        ret.push_back(
                path_data{pos, dist, std::abs(mean(i.volume)) * (1 << 20)});
    }
    return ret;
}

aligned::vector<aligned::vector<RayVisualisation::path_data>>
RayVisualisation::convert_to_path_data(
        const aligned::vector<aligned::vector<impulse>>& impulses,
        const glm::vec3& source) {
    aligned::vector<aligned::vector<path_data>> ret;
    ret.reserve(impulses.size());

    for (const auto& i : impulses) {
        ret.push_back(convert_to_path_data(i, source));
    }

    return ret;
}

aligned::vector<glm::vec3> RayVisualisation::extract_positions(
        const aligned::vector<aligned::vector<path_data>>& impulses,
        const glm::vec3& source) {
    aligned::vector<glm::vec3> ret;

    //  first in the buffer is the source position
    ret.push_back(source);

    //  now add positions ray-by-ray from the input
    for (const auto& ray : impulses) {
        for (const auto& reflection : ray) {
            ret.push_back(reflection.position);
        }
    }

    //  the last impulses.size() points will be moved to represent the wavefront
    ret.resize(ret.size() + impulses.size(), glm::vec3{});

    return ret;
}

aligned::vector<float> RayVisualisation::extract_pressures(
        const aligned::vector<aligned::vector<path_data>>& impulses) {
    aligned::vector<float> ret;

    //  source
    ret.push_back(1);

    //  impulses
    for (const auto& ray : impulses) {
        for (const auto& reflection : ray) {
            ret.push_back(reflection.pressure);
        }
    }

    //  the last impulses.size() points will be moved to represent the wavefront
    ret.resize(ret.size() + impulses.size(), 1);

    return ret;
}

aligned::vector<GLuint> RayVisualisation::compute_indices(
        const aligned::vector<aligned::vector<path_data>>& impulses,
        float time,
        size_t reflection_points) {
    aligned::vector<GLuint> ret;

    const auto distance = time * speed_of_sound;

    //  this will hold the begin index of each ray in the vertex buffer
    size_t counter = 1;
    const auto rays = impulses.size();
    for (size_t i = 0; i != rays; ++i, counter += impulses[i].size()) {
        const auto& ray = impulses[i];

        if (!ray.empty()) {
            ret.push_back(0);  //  source

            for (auto j{0u}; (j != ray.size() - 1) && (ray[j].distance < distance);
                 ++j) {
                const auto impulse_index = counter + j;
                ret.push_back(impulse_index);
                ret.push_back(impulse_index);
            }

            ret.push_back(reflection_points + 1 + i);
        }
    }

    return ret;
}

glm::vec3 RayVisualisation::ray_wavefront_position(
        const aligned::vector<path_data>& path,
        float time,
        const glm::vec3& source) {
    if (path.empty()) {
        return source;
    }

    const auto distance{time * speed_of_sound};
    const auto lim{path.end() - 1};
    auto it{path.begin()};
    for (; it != lim && it->distance < distance; ++it) {
        //  this line intentionally left blank
    }

    const auto far_node{*it};
    const auto near_node{it == path.begin() ? path_data{source, 0, 1}
                                            : *(it - 1)};

    const auto ratio{
            glm::clamp((distance - near_node.distance) /
                               (far_node.distance - near_node.distance),
                       0.0,
                       1.0)};

    return glm::mix(near_node.position, far_node.position, ratio);
}

aligned::vector<glm::vec3> RayVisualisation::ray_wavefront_position(
        const aligned::vector<aligned::vector<path_data>>& paths,
        float time,
        const glm::vec3& source) {
    return map_to_vector(paths, [&](const auto& i) {
        return ray_wavefront_position(i, time, source);
    });
}

//----------------------------------------------------------------------------//

RayShader::RayShader()
        : program(mglu::program::from_sources(vert, frag)) {}

void RayShader::set_model_matrix(const glm::mat4& m) const {
    program.set("v_model", m);
}
void RayShader::set_view_matrix(const glm::mat4& m) const {
    program.set("v_view", m);
}
void RayShader::set_projection_matrix(const glm::mat4& m) const {
    program.set("v_projection", m);
}

const char* RayShader::vert{R"(
#version 150
in vec3 v_position;
in float v_pressure;
out float f_pressure;

uniform mat4 v_model;
uniform mat4 v_view;
uniform mat4 v_projection;

void main() {
    vec4 modelview = v_view * v_model * vec4(v_position, 1.0);
    gl_Position = v_projection * modelview;

    f_pressure = v_pressure;
}
)"};

const char* RayShader::frag{R"(
#version 150
in float f_pressure;
out vec4 frag_color;

void main() {
    frag_color = vec4(vec3(f_pressure), 1.0);
}
)"};

//----------------------------------------------------------------------------//

RayVisualisation::RayVisualisation(
        const std::shared_ptr<RayShader>& shader,
        const aligned::vector<aligned::vector<impulse>>& impulses,
        const glm::vec3& source,
        const glm::vec3& receiver)
        : RayVisualisation(shader,
                           convert_to_path_data(impulses, source),
                           source,
                           receiver,
                           count(impulses)) {}

RayVisualisation::RayVisualisation(
        const std::shared_ptr<RayShader>& shader,
        const aligned::vector<aligned::vector<path_data>>& paths,
        const glm::vec3& source,
        const glm::vec3& receiver,
        size_t reflection_points)
        : shader(shader)
        , positions(extract_positions(paths, source))
        , pressures(extract_pressures(paths))
        , ibo(compute_indices(paths, 0, reflection_points))
        , source(source)
        , receiver(receiver)
        , paths(paths)
        , reflection_points(reflection_points) {
    const auto s_vao = vao.get_scoped();
    mglu::enable_and_bind_buffer(vao,
                                 positions,
                                 shader->get_attrib_location_v_position(),
                                 3,
                                 GL_FLOAT);
    mglu::enable_and_bind_buffer(vao,
                                 pressures,
                                 shader->get_attrib_location_v_pressure(),
                                 1,
                                 GL_FLOAT);
    ibo.bind();
}

void RayVisualisation::set_time(float t) {
    ibo.data(compute_indices(paths, t, reflection_points));
    positions.sub_data(1 + reflection_points,
                       ray_wavefront_position(paths, t, source));
}

void RayVisualisation::do_draw(const glm::mat4& modelview_matrix) const {
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(modelview_matrix);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_LINES, ibo.size(), GL_UNSIGNED_INT, nullptr);
}

glm::mat4 RayVisualisation::get_local_modelview_matrix() const {
    return glm::mat4{};
}