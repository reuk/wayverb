#pragma once

#include "common/cl/include.h"

#include "glm/glm.hpp"

template <typename T>
constexpr cl_float3 to_cl_float3(const T& t) {
    return cl_float3{{t.x, t.y, t.z, 0}};
}

template <typename T>
constexpr cl_int3 to_cl_int3(const T& t) {
    return cl_int3{{t.x, t.y, t.z, 0}};
}

template <typename T>
constexpr glm::vec3 to_vec3(const T& t) {
    return glm::vec3{t.x, t.y, t.z};
}

template <>
constexpr glm::vec3 to_vec3(const cl_float3& t) {
    return glm::vec3{t.s[0], t.s[1], t.s[2]};
}

template <typename T>
constexpr glm::ivec3 to_ivec3(const T& t) {
    return glm::ivec3{t.x, t.y, t.z};
}

template <>
constexpr glm::ivec3 to_ivec3(const cl_int3& t) {
    return glm::ivec3{t.s[0], t.s[1], t.s[2]};
}
