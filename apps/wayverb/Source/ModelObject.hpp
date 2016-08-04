#pragma once

#include "OtherComponents/BasicDrawableObject.hpp"
#include "OtherComponents/MoreConversions.hpp"

#include "common/scene_data.h"
#include "common/triangle.h"

namespace detail {
constexpr auto model_colour = 0.5;

inline aligned::vector<GLuint> get_indices(const scene_data& scene_data) {
    aligned::vector<GLuint> ret(scene_data.get_triangles().size() * 3);
    auto count = 0u;
    for (const auto& tri : scene_data.get_triangles()) {
        ret[count + 0] = tri.v0;
        ret[count + 1] = tri.v1;
        ret[count + 2] = tri.v2;
        count += 3;
    }
    return ret;
}
}  // namespace detail

class ModelObject final : public BasicDrawableObject {
public:
    template <typename T>
    ModelObject(T& shader, const scene_data& scene_data)
            : BasicDrawableObject(shader,
                                  scene_data.get_converted_vertices(),
                                  aligned::vector<glm::vec4>(
                                          scene_data.get_vertices().size(),
                                          glm::vec4(detail::model_colour,
                                                    detail::model_colour,
                                                    detail::model_colour,
                                                    detail::model_colour)),
                                  detail::get_indices(scene_data),
                                  GL_TRIANGLES) {}
};
