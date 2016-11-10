#pragma once

#include "core/cl/include.h"
#include "core/cl/representation.h"

namespace wayverb {
namespace core {
struct alignas(1 << 3) triangle final {
    cl_uint surface;
    cl_uint v0;
    cl_uint v1;
    cl_uint v2;
};

template <>
struct cl_representation<triangle> final {
    static constexpr auto value = R"(
typedef struct {
    uint surface;
    uint v0;
    uint v1;
    uint v2;
} triangle;
)";
};

constexpr bool operator==(const triangle& a, const triangle& b) {
    return a.surface == b.surface && a.v0 == b.v0 && a.v1 == b.v1 &&
           a.v2 == b.v2;
}

constexpr bool operator!=(const triangle& a, const triangle& b) {
    return !(a == b);
}

}  // namespace core
}  // namespace wayverb
