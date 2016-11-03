#include "MeshObject.hpp"

#include "waveguide/waveguide.h"

#include "utilities/popcount.h"

#include "glm/gtc/random.hpp"

MeshObject::MeshObject(const std::shared_ptr<MeshShader>& shader,
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

void MeshObject::do_draw(const glm::mat4& modelview_matrix) const {
    //    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(modelview_matrix);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_POINTS, ibo.size(), GL_UNSIGNED_INT, nullptr);
    glDepthMask(GL_TRUE);
    //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

glm::mat4 MeshObject::get_local_modelview_matrix() const { return glm::mat4{}; }

void MeshObject::set_pressures(const util::aligned::vector<float>& u) {
    pressures.data(u);
}

void MeshObject::zero_pressures() {
    set_pressures(util::aligned::vector<float>(ibo.size(), 0));
}

void MeshObject::set_positions(const util::aligned::vector<glm::vec3>& positions) {
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
    return get_direction_vector(i & wayverb::waveguide::id_nx) + get_direction_vector(i & wayverb::waveguide::id_px) +
           get_direction_vector(i & wayverb::waveguide::id_ny) + get_direction_vector(i & wayverb::waveguide::id_py) +
           get_direction_vector(i & wayverb::waveguide::id_nz) + get_direction_vector(i & wayverb::waveguide::id_pz);
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
                            i.boundary_type != wayverb::waveguide::id_reentrant) {
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
                            case wayverb::waveguide::id_inside: return glm::vec4{0, 0, 0, 0};
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
        if (node.boundary_type != wayverb::waveguide::id_none && node.boundary_type != wayverb::waveguide::id_inside) {
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

void DebugMeshObject::do_draw(const glm::mat4& modelview_matrix) const {
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(modelview_matrix);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_POINTS, ibo.size(), GL_UNSIGNED_INT, nullptr);
}

glm::mat4 DebugMeshObject::get_local_modelview_matrix() const {
    return glm::mat4{};
}
