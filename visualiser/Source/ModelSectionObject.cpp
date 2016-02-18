#include "ModelSectionObject.hpp"

#include "MoreConversions.hpp"

static constexpr auto model_colour = 0.5;

ModelSectionObject::ModelSectionObject(const GenericShader &shader,
                                       const SceneData &scene_data,
                                       const Octree &octree)
        : BasicDrawableObject(
              shader,
              get_vertices(scene_data),
              std::vector<glm::vec4>(
                  scene_data.get_vertices().size(),
                  glm::vec4(
                      model_colour, model_colour, model_colour, model_colour)),
              get_indices(scene_data, octree),
              GL_TRIANGLES)
        , octree(shader, octree) {
}

std::vector<glm::vec3> ModelSectionObject::get_vertices(
    const SceneData &scene_data) const {
    std::vector<glm::vec3> ret(scene_data.get_vertices().size());
    std::transform(scene_data.get_vertices().begin(),
                   scene_data.get_vertices().end(),
                   ret.begin(),
                   [](auto i) { return to_glm_vec3(i); });
    return ret;
}

void fill_indices_vector(const SceneData &scene_data,
                         const Octree &octree,
                         std::vector<GLuint> &ret) {
    for (const auto &i : octree.get_triangles()) {
        const auto &tri = scene_data.get_triangles()[i];
        ret.push_back(tri.v0);
        ret.push_back(tri.v1);
        ret.push_back(tri.v2);
    }

    for (const auto &i : octree.get_nodes()) {
        fill_indices_vector(scene_data, i, ret);
    }
}

std::vector<GLuint> ModelSectionObject::get_indices(
    const SceneData &scene_data, const Octree &octree) const {
    std::vector<GLuint> ret;
    fill_indices_vector(scene_data, octree, ret);
    return ret;
}

void ModelSectionObject::draw() const {
    BasicDrawableObject::draw();
    octree.draw();
}

ModelSectionObject::DrawableOctree::DrawableOctree(const GenericShader &shader,
                                                   const Octree &octree)
        : shader(shader)
        , do_draw(!octree.get_triangles().empty() && !octree.has_nodes())
        , aabb(octree.get_aabb()) {
    if (do_draw) {
        for (const auto &i : octree.get_nodes()) {
            nodes.emplace_back(shader, i);
        }
    }
}

void ModelSectionObject::DrawableOctree::draw_worker(BoxObject &box) const {
    if (do_draw) {
        box.set_scale(to_glm_vec3(aabb.get_dimensions()));
        box.set_position(to_glm_vec3(aabb.get_centre()));
        box.draw();
        for (const auto &i : nodes)
            i.draw_worker(box);
    }
}

void ModelSectionObject::DrawableOctree::draw() const {
    BoxObject box(shader);
    draw_worker(box);
}