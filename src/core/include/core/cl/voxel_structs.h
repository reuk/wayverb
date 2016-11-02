#pragma once

#include "core/cl/representation.h"
#include "core/cl/traits.h"

namespace wayverb {
namespace core {

struct alignas(1 << 4) aabb final {
    cl_float3 c0;
    cl_float3 c1;
};

template <>
struct cl_representation<aabb> final {
    static constexpr auto value = R"(
typedef struct {
    float3 c0;
    float3 c1;
} aabb;
)";
};

constexpr auto to_tuple(const aabb& x) { return std::tie(x.c0, x.c1); }

constexpr bool operator==(const aabb& a, const aabb& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const aabb& a, const aabb& b) { return !(a == b); }

}  // namespace core
}  // namespace wayverb
