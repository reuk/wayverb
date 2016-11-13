#include "reflections_object.h"

#include "core/conversions.h"

#include "utilities/map_to_vector.h"

#include <numeric>

namespace {

class distance_accumulator final {
public:
    distance_accumulator(const glm::vec3& original_position)
            : last_position{original_position} {}

    float operator()(const glm::vec3& i) {
        current_distance += glm::distance(last_position, i);
        last_position = i;
        return current_distance;
    }

private:
    glm::vec3 last_position;
    float current_distance = 0;
};

template <typename T>
size_t count(const T&) {
    return 1;
}

template <typename T>
size_t count(const util::aligned::vector<T>& t) {
    return std::accumulate(
            t.begin(), t.end(), size_t{0}, [](const auto& i, const auto& j) {
                return i + count(j);
            });
}

}  // namespace

util::aligned::vector<reflections_object::path_data>
reflections_object::convert_to_path_data(
        const util::aligned::vector<wayverb::raytracer::reflection>&
                reflections,
        const glm::vec3& source) {
    util::aligned::vector<path_data> ret;
    ret.reserve(reflections.size());

    distance_accumulator d{source};
    for (const auto& i : reflections) {
        const auto pos = wayverb::core::to_vec3{}(i.position);
        const auto dist = d(pos);
        ret.emplace_back(path_data{pos, dist});
    }
    return ret;
}

util::aligned::vector<util::aligned::vector<reflections_object::path_data>>
reflections_object::convert_to_path_data(
        const util::aligned::vector<util::aligned::vector<
                wayverb::raytracer::reflection>>& reflections,
        const glm::vec3& source) {
    util::aligned::vector<util::aligned::vector<path_data>> ret;
    ret.reserve(reflections.size());

    for (const auto& i : reflections) {
        ret.emplace_back(convert_to_path_data(i, source));
    }

    return ret;
}

util::aligned::vector<glm::vec3> reflections_object::extract_positions(
        const util::aligned::vector<util::aligned::vector<path_data>>&
                reflections,
        const glm::vec3& source) {
    util::aligned::vector<glm::vec3> ret;

    //  first in the buffer is the source position
    ret.emplace_back(source);

    //  now add positions ray-by-ray from the input
    for (const auto& ray : reflections) {
        for (const auto& r : ray) {
            ret.emplace_back(r.position);
        }
    }

    //  the last reflections.size() points will be moved to represent the
    //  wavefront
    ret.resize(ret.size() + reflections.size(), glm::vec3{});

    return ret;
}

util::aligned::vector<float> reflections_object::extract_pressures(
        const util::aligned::vector<util::aligned::vector<path_data>>&
                reflections) {
    util::aligned::vector<float> ret;

    //  source
    ret.emplace_back(1);

    //  reflections
    for (const auto& ray : reflections) {
        for (const auto& r : ray) {
            //  TODO find a better way of displaying energy
            ret.emplace_back(1);
        }
    }

    //  the last reflections.size() points will be moved to represent the
    //  wavefront
    ret.resize(ret.size() + reflections.size(), 1);

    return ret;
}

util::aligned::vector<GLuint> reflections_object::compute_indices(
        const util::aligned::vector<util::aligned::vector<path_data>>&
                reflections,
        double distance,
        size_t reflection_points) {
    util::aligned::vector<GLuint> ret;

    //  this will hold the begin index of each ray in the vertex buffer
    size_t counter = 1;
    const auto rays = reflections.size();
    for (size_t i = 0; i != rays; ++i, counter += reflections[i].size()) {
        const auto& ray = reflections[i];

        if (!ray.empty()) {
            ret.emplace_back(0);  //  source

            for (auto j{0u};
                 (j != ray.size() - 1) && (ray[j].distance < distance);
                 ++j) {
                const auto reflection_index = counter + j;
                ret.emplace_back(reflection_index);
                ret.emplace_back(reflection_index);
            }

            ret.emplace_back(reflection_points + 1 + i);
        }
    }

    return ret;
}

glm::vec3 reflections_object::ray_wavefront_position(
        const util::aligned::vector<path_data>& path,
        double distance,
        const glm::vec3& source) {
    if (path.empty()) {
        return source;
    }

    const auto lim{path.end() - 1};
    auto it{path.begin()};
    for (; it != lim && it->distance < distance; ++it) {
        //  this line intentionally left blank
    }

    const auto far_node{*it};
    const auto near_node{it == path.begin() ? path_data{source, 0} : *(it - 1)};

    const auto ratio{
            glm::clamp((distance - near_node.distance) /
                               (far_node.distance - near_node.distance),
                       0.0,
                       1.0)};

    return glm::mix(near_node.position, far_node.position, ratio);
}

util::aligned::vector<glm::vec3> reflections_object::ray_wavefront_position(
        const util::aligned::vector<util::aligned::vector<path_data>>& paths,
        double distance,
        const glm::vec3& source) {
    return util::map_to_vector(begin(paths), end(paths), [&](const auto& i) {
        return ray_wavefront_position(i, distance, source);
    });
}

////////////////////////////////////////////////////////////////////////////////

reflections_shader::reflections_shader()
        : program_{mglu::program::from_sources(vert, frag)} {}

void reflections_shader::set_model_matrix(const glm::mat4& m) const {
    program_.set("v_model", m);
}

void reflections_shader::set_view_matrix(const glm::mat4& m) const {
    program_.set("v_view", m);
}

void reflections_shader::set_projection_matrix(const glm::mat4& m) const {
    program_.set("v_projection", m);
}

const char* reflections_shader::vert = R"(
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
)";

const char* reflections_shader::frag = R"(
#version 150
in float f_pressure;
out vec4 frag_color;

void main() {
    frag_color = vec4(vec3(f_pressure), 1.0);
}
)";

////////////////////////////////////////////////////////////////////////////////

reflections_object::reflections_object(
        const std::shared_ptr<reflections_shader>& shader,
        const util::aligned::vector<
                util::aligned::vector<wayverb::raytracer::reflection>>&
                reflections,
        const glm::vec3& source)
        : reflections_object(shader,
                             convert_to_path_data(reflections, source),
                             source,
                             count(reflections)) {}

reflections_object::reflections_object(
        const std::shared_ptr<reflections_shader>& shader,
        const util::aligned::vector<util::aligned::vector<path_data>>& paths,
        const glm::vec3& source,
        size_t reflection_points)
        : shader_{shader}
        , positions_{extract_positions(paths, source)}
        , pressures_{extract_pressures(paths)}
        , ibo_{compute_indices(paths, 0, reflection_points)}
        , source_{source}
        , paths_{paths}
        , reflection_points_{reflection_points} {
    const auto s_vao = vao_.get_scoped();
    mglu::enable_and_bind_buffer(vao_,
                                 positions_,
                                 shader->get_attrib_location_v_position(),
                                 3,
                                 GL_FLOAT);
    mglu::enable_and_bind_buffer(vao_,
                                 pressures_,
                                 shader->get_attrib_location_v_pressure(),
                                 1,
                                 GL_FLOAT);
    ibo_.bind();
}

void reflections_object::set_distance(double t) {
    ibo_.data(compute_indices(paths_, t, reflection_points_));
    positions_.sub_data(1 + reflection_points_,
                        ray_wavefront_position(paths_, t, source_));
}

void reflections_object::do_draw(const glm::mat4& model_matrix) const {
    if (const auto shader = shader_.lock()) {
        auto s_shader = shader->get_scoped();
        shader->set_model_matrix(model_matrix);

        auto s_vao = vao_.get_scoped();
        glDrawElements(GL_LINES, ibo_.size(), GL_UNSIGNED_INT, nullptr);
    }
}

glm::mat4 reflections_object::get_local_model_matrix() const {
    return glm::mat4{};
}
