#pragma once

#include "common/cl/representation.h"
#include "common/cl/traits.h"

using volume_type = cl_float8;

template <>
struct cl_representation<volume_type> final {
    static constexpr auto value{R"(
typedef float8 volume_type;
)"};
};

constexpr auto make_volume_type(float f) {
    return volume_type{{f, f, f, f, f, f, f, f}};
}

//----------------------------------------------------------------------------//

struct alignas(1 << 5) surface {
    volume_type specular_absorption{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
    volume_type diffuse_coefficient{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
};

template <>
struct cl_representation<surface> final {
    static constexpr auto value{R"(
typedef struct {
    volume_type specular_absorption;
    volume_type diffuse_coefficient;
} surface;
)"};
};

constexpr auto make_surface(float s, float d) {
    return surface{make_volume_type(s), make_volume_type(d)};
}

inline auto get_specular_absorption(const surface& s) {
    return s.specular_absorption;
}

//----------------------------------------------------------------------------//

struct alignas(1 << 3) triangle final {
    cl_uint surface;
    cl_uint v0;
    cl_uint v1;
    cl_uint v2;
};

template <>
struct cl_representation<triangle> final {
    static constexpr auto value{R"(
typedef struct {
    uint surface;
    uint v0;
    uint v1;
    uint v2;
} triangle;
)"};
};

constexpr auto to_tuple(const triangle& x) {
    return std::tie(x.surface, x.v0, x.v1, x.v2);
}

constexpr bool operator==(const triangle& a, const triangle& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const triangle& a, const triangle& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

struct alignas(1 << 4) triangle_verts final {
    cl_float3 v0;
    cl_float3 v1;
    cl_float3 v2;
};

template <>
struct cl_representation<triangle_verts> final {
    static constexpr auto value{R"(
typedef struct {
    float3 v0;
    float3 v1;
    float3 v2;
} triangle_verts;
)"};
};

constexpr auto to_tuple(const triangle_verts& x) {
    return std::tie(x.v0, x.v1, x.v2);
}

constexpr bool operator==(const triangle_verts& a, const triangle_verts& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const triangle_verts& a, const triangle_verts& b) {
    return !(a == b);
}
