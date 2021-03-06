#pragma once

#include "core/almost_equal.h"
#include "core/cl/scene_structs.h"

#include <string>

namespace wayverb {
namespace core {

struct alignas(1 << 4) ray final {
    cl_float3 position;
    cl_float3 direction;
};

template <>
struct cl_representation<ray> final {
    static constexpr auto value = R"(
typedef struct {
    float3 position;
    float3 direction;
} ray;
)";
};

constexpr auto to_tuple(const ray& x) {
    return std::tie(x.position, x.direction);
}

constexpr bool operator==(const ray& a, const ray& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const ray& a, const ray& b) { return !(a == b); }

////////////////////////////////////////////////////////////////////////////////

struct alignas(1 << 2) triangle_inter final {
    cl_float t;  //  the distance along the ray to the intersection
    cl_float u;  //  barycentric coordinate u
    cl_float v;  //  barycentric coordinate v
};

template <>
struct cl_representation<triangle_inter> final {
    static constexpr auto value = R"(
typedef struct {
    float t;
    float u;
    float v;
} triangle_inter;
)";
};

constexpr bool operator==(const triangle_inter& a, const triangle_inter& b) {
    return std::tie(a.t, a.u, a.v) == std::tie(b.t, b.u, b.v);
}

constexpr bool operator!=(const triangle_inter& a, const triangle_inter& b) {
    return !(a == b);
}

constexpr bool is_degenerate(const triangle_inter& i) {
    const auto ulp = 10;
    return almost_equal(i.u, 0.0f, ulp) || almost_equal(i.v, 0.0f, ulp) ||
           almost_equal(i.u + i.v, 1.0f, ulp);
}

////////////////////////////////////////////////////////////////////////////////

struct alignas(1 << 3) intersection final {
    triangle_inter inter;
    cl_uint index;
};

template <>
struct cl_representation<intersection> final {
    static constexpr auto value = R"(
typedef struct {
    triangle_inter inter;
    uint index;
} intersection;
)";
};

constexpr auto to_tuple(const intersection& x) {
    return std::tie(x.inter, x.index);
}

constexpr bool operator==(const intersection& a, const intersection& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const intersection& a, const intersection& b) {
    return !(a == b);
}

}  // namespace core
}  // namespace wayverb
