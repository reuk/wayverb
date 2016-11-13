#include "mesh_object.h"

#include "waveguide/waveguide.h"

#include "utilities/popcount.h"

#include "glm/gtc/random.hpp"

mesh_shader::mesh_shader()
        : program_{mglu::program::from_sources(vertex_shader,
                                               fragment_shader)} {}

void mesh_shader::set_model_matrix(const glm::mat4& mat) const {
    program_.set("v_model", mat);
}
void mesh_shader::set_view_matrix(const glm::mat4& mat) const {
    program_.set("v_view", mat);
}
void mesh_shader::set_projection_matrix(const glm::mat4& mat) const {
    program_.set("v_projection", mat);
}

const char* mesh_shader::vertex_shader = R"(
#version 150
in vec3 v_position;
in vec4 v_colour;
out vec4 f_color;
uniform mat4 v_model;
uniform mat4 v_view;
uniform mat4 v_projection;

const float min_point = 0.1;
const float max_point = 8.0;
const float min_color = 0.9;
const float max_color = 1.0;
const float max_dist = 20.0;

void main() {
    vec4 cs_position = v_view * v_model * vec4(v_position, 1.0);
    gl_Position = v_projection * cs_position;

    float dist = -cs_position.z;
    float scaled = 1.0 - (dist / max_dist);
    float p = clamp(scaled * (max_point - min_point) + min_point, min_point, max_point);
    float c = clamp(scaled * (max_color - min_color) + min_color, min_color, max_color);
    gl_PointSize = p;
    f_color = v_colour;
}
)";

const char* mesh_shader::fragment_shader = R"(
#version 150
in vec4 f_color;
out vec4 frag_color;
void main() {
    frag_color = f_color;
}
)";

////////////////////////////////////////////////////////////////////////////////

mesh_object::mesh_object(const std::shared_ptr<mesh_shader>& shader,
                         const glm::vec3* positions,
                         size_t num_positions)
        : shader_{shader} {
    std::vector<GLuint> indices(num_positions);
    std::iota(indices.begin(), indices.end(), 0);
    ibo.data(indices);

    geometry_.data(positions, num_positions);

    {
        std::vector<glm::vec4> colours(num_positions, glm::vec4{});
        set_colours(colours.data(), colours.size());
    }

    //  init vao
    const auto s_vao = vao_.get_scoped();
    mglu::enable_and_bind_buffer(vao_,
                                 geometry_,
                                 shader->get_attrib_location_v_position(),
                                 3,
                                 GL_FLOAT);

    mglu::enable_and_bind_buffer(vao_,
                                 colours_,
                                 shader->get_attrib_location_v_colour(),
                                 1,
                                 GL_FLOAT);
    ibo.bind();
}

void mesh_object::set_colours(const glm::vec4* colours, size_t num_colours) {
    colours_.data(colours, num_colours);
}

void mesh_object::do_draw(const glm::mat4& matrix) const {
    if (const auto shader = shader_.lock()) {
        glDepthMask(GL_FALSE);
        auto s_shader = shader->get_scoped();
        shader->set_model_matrix(matrix);

        auto s_vao = vao_.get_scoped();
        glDrawElements(GL_POINTS, ibo.size(), GL_UNSIGNED_INT, nullptr);
        glDepthMask(GL_TRUE);
    }
}

glm::mat4 mesh_object::get_local_model_matrix() const { return glm::mat4{}; }

/*

////////////////////////////////////////////////////////////////////////////////

MeshObject::MeshObject(const std::shared_ptr<mesh_shader>& shader,
                       const util::aligned::vector<glm::vec3>& positions)
        : shader(shader) {
    set_positions(positions);

    //  init vao
    const auto s_vao = vao.get_scoped();
    mglu::enable_and_bind_buffer(vao,
                                 geometry,
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

void MeshObject::do_draw(const glm::mat4& model_matrix) const {
    //    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(model_matrix);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_POINTS, ibo.size(), GL_UNSIGNED_INT, nullptr);
    glDepthMask(GL_TRUE);
    //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

glm::mat4 MeshObject::get_local_model_matrix() const { return glm::mat4{}; }

void MeshObject::set_pressures(const util::aligned::vector<float>& u) {
    pressures.data(u);
}

void MeshObject::zero_pressures() {
    set_pressures(util::aligned::vector<float>(ibo.size(), 0));
}

void MeshObject::set_positions(
        const util::aligned::vector<glm::vec3>& positions) {
    util::aligned::vector<GLuint> indices(positions.size());
    std::iota(indices.begin(), indices.end(), 0);
    ibo.data(indices);

    geometry.data(positions);
    zero_pressures();
}

//----------------------------------------------------------------------------//
namespace {
auto get_direction_vector(int i) {
    switch (i) {
        case wayverb::waveguide::id_nx: return glm::vec3{-1, 0, 0};
        case wayverb::waveguide::id_px: return glm::vec3{1, 0, 0};
        case wayverb::waveguide::id_ny: return glm::vec3{0, -1, 0};
        case wayverb::waveguide::id_py: return glm::vec3{0, 1, 0};
        case wayverb::waveguide::id_nz: return glm::vec3{0, 0, -1};
        case wayverb::waveguide::id_pz: return glm::vec3{0, 0, 1};
        default: return glm::vec3{0};
    }
}
auto get_summed_direction_vector(int i) {
    return get_direction_vector(i & wayverb::waveguide::id_nx) +
           get_direction_vector(i & wayverb::waveguide::id_px) +
           get_direction_vector(i & wayverb::waveguide::id_ny) +
           get_direction_vector(i & wayverb::waveguide::id_py) +
           get_direction_vector(i & wayverb::waveguide::id_nz) +
           get_direction_vector(i & wayverb::waveguide::id_pz);
}
}  // namespace

DebugMeshObject::DebugMeshObject(
        const std::shared_ptr<mglu::generic_shader>& shader,
        const wayverb::waveguide::mesh& model,
        mode m)
        : shader(shader) {
    geometry.data(compute_node_positions(model.get_descriptor()));

    switch (m) {
        case mode::closest_surface: {
            //  count surfaces
            const auto num_surfaces{
                    model.get_structure().get_coefficients().size()};

            //  generate colour for each surface
            util::aligned::vector<glm::vec4> surf_colours(num_surfaces);
            std::generate(begin(surf_colours), end(surf_colours), [] {
                return glm::vec4{glm::linearRand(glm::vec3{0}, glm::vec3{1}),
                                 1.0};
            });

            //  for each 1d node, set the colour appropriately
            colours.data(util::map_to_vector(
                    begin(model.get_structure().get_condensed_nodes()),
                    end(model.get_structure().get_condensed_nodes()),
                    [&](const auto& i) {
                        if (util::popcount(i.boundary_type) == 1 &&
                            i.boundary_type != wayverb::waveguide::id_inside &&
                            i.boundary_type !=
                                    wayverb::waveguide::id_reentrant) {
                            const auto ind{i.boundary_index};
                            const auto surf{
                                    model.get_structure()
                                            .get_boundary_indices<1>()[ind]
                                            .array[0]};
                            return surf_colours.at(surf);
                        }
                        return glm::vec4{};
                    }));
            break;
        }
        case mode::boundary_type: {
            colours.data(util::map_to_vector(
                    begin(model.get_structure().get_condensed_nodes()),
                    end(model.get_structure().get_condensed_nodes()),
                    [](const auto& i) {
                        switch (i.boundary_type) {
                            case wayverb::waveguide::id_none:
                            case wayverb::waveguide::id_inside:
                                return glm::vec4{0, 0, 0, 0};
                            default:
                                return glm::vec4{
                                        glm::abs(get_summed_direction_vector(
                                                i.boundary_type)),
                                        1.0};
                        }
                    }));
            break;
        }
    }

    util::aligned::vector<GLuint> indices;
    const auto lim{model.get_structure().get_condensed_nodes().size()};
    for (auto i{0u}; i != lim; ++i) {
        const auto& node{model.get_structure().get_condensed_nodes()[i]};
        if (node.boundary_type != wayverb::waveguide::id_none &&
            node.boundary_type != wayverb::waveguide::id_inside) {
            indices.push_back(i);
        }
    }
    ibo.data(indices);

    const auto s_vao = vao.get_scoped();
    mglu::enable_and_bind_buffer(vao,
                                 geometry,
                                 shader->get_attrib_location_v_position(),
                                 3,
                                 GL_FLOAT);
    mglu::enable_and_bind_buffer(
            vao, colours, shader->get_attrib_location_v_color(), 4, GL_FLOAT);
    ibo.bind();
}

void DebugMeshObject::do_draw(const glm::mat4& model_matrix) const {
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(model_matrix);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_POINTS, ibo.size(), GL_UNSIGNED_INT, nullptr);
}

glm::mat4 DebugMeshObject::get_local_model_matrix() const {
    return glm::mat4{};
}
*/
