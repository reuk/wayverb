#include "ModelObject.hpp"

#include "MoreConversions.hpp"

static constexpr auto model_colour = 0.5;

ModelObject::ModelObject(const GenericShader &shader,
                         const SceneData &scene_data)
        : BasicDrawableObject(
              shader,
              get_vertices(scene_data),
              std::vector<glm::vec4>(
                  scene_data.get_vertices().size(),
                  glm::vec4(
                      model_colour, model_colour, model_colour, model_colour)),
              get_indices(scene_data),
              GL_TRIANGLES) {
}

std::vector<glm::vec3> ModelObject::get_vertices(
    const SceneData &scene_data) const {
    std::vector<glm::vec3> ret(scene_data.get_vertices().size());
    std::transform(scene_data.get_vertices().begin(),
                   scene_data.get_vertices().end(),
                   ret.begin(),
                   [](auto i) { return to_glm_vec3(i); });
    return ret;
}
std::vector<GLuint> ModelObject::get_indices(
    const SceneData &scene_data) const {
    std::vector<GLuint> ret(scene_data.get_triangles().size() * 3);
    auto count = 0u;
    for (const auto &tri : scene_data.get_triangles()) {
        ret[count + 0] = tri.v0;
        ret[count + 1] = tri.v1;
        ret[count + 2] = tri.v2;
        count += 3;
    }
    return ret;
}