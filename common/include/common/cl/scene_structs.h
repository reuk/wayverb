#pragma once

#include "common/cl/cl_representation.h"
#include "common/cl_traits.h"

using volume_type = cl_float8;

template <>
struct cl_representation<volume_type> final {
    static constexpr const char* value{R"(
#ifndef VOLUME_TYPE_DEFINITION__
#define VOLUME_TYPE_DEFINITION__
typedef float8 volume_type;
#endif
)"};
};

//----------------------------------------------------------------------------//

struct alignas(1 << 5) surface {
    volume_type specular{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
    volume_type diffuse{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
};

template <>
struct cl_representation<surface> final {
    static constexpr const char* value{R"(
#ifndef SURFACE_DEFINITION__
#define SURFACE_DEFINITION__
typedef struct {
    volume_type specular;
    volume_type diffuse;
} surface;
#endif
)"};
};

//----------------------------------------------------------------------------//

struct alignas(1 << 3) triangle final {
    cl_ulong surface;
    cl_ulong v0;
    cl_ulong v1;
    cl_ulong v2;
};

template <>
struct cl_representation<triangle> final {
    static constexpr const char* value{R"(
#ifndef TRIANGLE_DEFINITION__
#define TRIANGLE_DEFINITION__
typedef struct {
    ulong surface;
    ulong v0;
    ulong v1;
    ulong v2;
} triangle;
#endif
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
    static constexpr const char* value{R"(
#ifndef TRIANGLE_VERTS_DEFINITION__
#define TRIANGLE_VERTS_DEFINITION__
typedef struct {
    float3 v0;
    float3 v1;
    float3 v2;
} triangle_verts;
#endif
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
