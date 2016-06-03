#include "ModelObject.hpp"

#include "common/triangle.h"

#include "MoreConversions.hpp"

static constexpr auto model_colour = 0.5;

ModelObject::ModelObject(const GenericShader &shader,
                         const SceneData &scene_data)
        : BasicDrawableObject(
              shader,
              scene_data.get_converted_vertices(),
              std::vector<glm::vec4>(
                  scene_data.get_vertices().size(),
                  glm::vec4(
                      model_colour, model_colour, model_colour, model_colour)),
              get_indices(scene_data),
              GL_TRIANGLES) {
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