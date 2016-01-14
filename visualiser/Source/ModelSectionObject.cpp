#include "ModelSectionObject.hpp"

static constexpr auto model_colour = 0.5;

ModelSectionObject::ModelSectionObject(const GenericShader &shader,
                                       const SceneData &scene_data,
                                       const Octree &octree)
        : BasicDrawableObject(
              shader,
              get_vertices(scene_data),
              std::vector<glm::vec4>(
                  scene_data.vertices.size(),
                  glm::vec4(
                      model_colour, model_colour, model_colour, model_colour)),
              get_indices(scene_data, octree),
              GL_TRIANGLES)
        , octree(octree) {
}

std::vector<glm::vec3> ModelSectionObject::get_vertices(
    const SceneData &scene_data) const {
    std::vector<glm::vec3> ret(scene_data.vertices.size());
    std::transform(scene_data.vertices.begin(),
                   scene_data.vertices.end(),
                   ret.begin(),
                   [](auto i) { return glm::vec3(i.x, i.y, i.z); });
    return ret;
}

void fill_indices_vector(const SceneData &scene_data,
                         const Octree &octree,
                         std::vector<GLuint> &ret) {
    for (const auto &i : octree.get_triangles()) {
        const auto &tri = scene_data.triangles[i];
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

void ModelSectionObject::draw_octree(const Octree &octree,
                                     BoxObject &box) const {
    if (!octree.get_triangles().empty() && octree.get_nodes().empty()) {
        const auto &aabb = octree.get_aabb();
        box.set_scale(to_glm_vec3(aabb.get_dimensions()));
        box.set_position(to_glm_vec3(aabb.get_centre()));
        box.draw();
    }

    for (const auto &i : octree.get_nodes()) {
        draw_octree(i, box);
    }
}

void ModelSectionObject::draw() const {
    BasicDrawableObject::draw();
    BoxObject box(get_shader());
    draw_octree(octree, box);
}