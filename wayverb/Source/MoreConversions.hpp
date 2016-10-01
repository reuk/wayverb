#pragma once

#include <glm/glm.hpp>

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

template <typename T>
inline auto to_glm_vec3(const T& t) {
    return glm::vec3(t.x, t.y, t.z);
}

template <>
inline auto to_glm_vec3(const cl_float3& t) {
    return glm::vec3(t.s[0], t.s[1], t.s[2]);
}
