#pragma once

#include "common/cl/cl_representation.h"
#include "common/cl_traits.h"

struct alignas(1 << 4) aabb final {
    cl_float3 c0;
    cl_float3 c1;
};

template <>
struct cl_representation<aabb> final {
    static constexpr const char* value{R"(
#ifndef AABB_DEFINITION__
#define AABB_DEFINITION__
typedef struct {
    float3 c0;
    float3 c1;
} aabb;
#endif
)"};
};

constexpr auto to_tuple(const aabb& x) { return std::tie(x.c0, x.c1); }

constexpr bool operator==(const aabb& a, const aabb& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const aabb& a, const aabb& b) { return !(a == b); }
