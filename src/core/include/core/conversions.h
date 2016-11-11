#pragma once

#include "core/cl/include.h"

#include "glm/glm.hpp"

namespace wayverb {
namespace core {

struct to_cl_float3 final {
    template <typename T>
    constexpr cl_float3 operator()(const T& t) const {
        return cl_float3{{t.x, t.y, t.z, 0}};
    }
};

struct to_cl_int3 final {
    template <typename T>
    constexpr cl_int3 operator()(const T& t) const {
        return cl_int3{{t.x, t.y, t.z, 0}};
    }
};

////////////////////////////////////////////////////////////////////////////////

struct to_vec3 final {
    template <typename T>
    constexpr glm::vec3 operator()(const T& t) const {
        return glm::vec3{t.x, t.y, t.z};
    }
};

template <>
constexpr glm::vec3 to_vec3::operator()(const cl_float3& t) const {
    return glm::vec3{t.s[0], t.s[1], t.s[2]};
}

template <>
constexpr glm::vec3 to_vec3::operator()(const glm::vec3& t) const {
    return t;
}

struct to_ivec3 final {
    template <typename T>
    constexpr glm::ivec3 operator()(const T& t) const {
        return glm::ivec3{t.x, t.y, t.z};
    }
};

template <>
constexpr glm::ivec3 to_ivec3::operator()(const cl_int3& t) const {
    return glm::ivec3{t.s[0], t.s[1], t.s[2]};
}

template <>
constexpr glm::ivec3 to_ivec3::operator()(const glm::ivec3& t) const {
    return t;
}

////////////////////////////////////////////////////////////////////////////////

struct to_vec2 final {
    template <typename T>
    constexpr glm::vec2 operator()(const T& t) const {
        return glm::vec2{t.x, t.y};
    }
};

template<>
constexpr glm::vec2 to_vec2::operator()(const cl_float2& t) const {
    return glm::vec2{t.s[0], t.s[1]};
}

template <>
constexpr glm::vec2 to_vec2::operator()(const glm::vec2& t) const {
    return t;
}

struct to_ivec2 final {
    template <typename T>
    constexpr glm::ivec2 operator()(const T& t) const {
        return glm::ivec2{t.x, t.y};
    }
};

template<>
constexpr glm::ivec2 to_ivec2::operator()(const cl_int2& t) const {
    return glm::ivec2{t.s[0], t.s[1]};
}

template <>
constexpr glm::ivec2 to_ivec2::operator()(const glm::ivec2& t) const {
    return t;
}

}  // namespace core
}  // namespace wayverb
