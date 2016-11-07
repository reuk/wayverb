#pragma once

#include "BasicDrawableObject.hpp"

#include "combined/engine.h"

#include "core/scene_data.h"

namespace detail {
constexpr auto model_colour = 0.5;

inline util::aligned::vector<GLuint> get_indices(const wayverb::core::gpu_scene_data& scene_data) {
    util::aligned::vector<GLuint> ret;
    ret.reserve(scene_data.get_triangles().size() * 3);
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
    ModelObject(T& shader, const wayverb::core::gpu_scene_data& scene_data)
            : BasicDrawableObject(shader,
                                  wayverb::core::convert(scene_data.get_vertices()),
                                  util::aligned::vector<glm::vec4>(
                                          scene_data.get_vertices().size(),
                                          glm::vec4(detail::model_colour,
                                                    detail::model_colour,
                                                    detail::model_colour,
                                                    detail::model_colour)),
                                  detail::get_indices(scene_data),
                                  GL_TRIANGLES) {}
};
