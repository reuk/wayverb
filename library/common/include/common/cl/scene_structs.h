#pragma once

#include "common/cl/representation.h"
#include "common/cl/traits.h"

/// If you change this, remember to update the cl_representation for bands_type
constexpr auto simulation_bands = 8;

using bands_type = detail::cl_vector_constructor_t<float, simulation_bands>;

template <>
struct cl_representation<bands_type> final {
    static constexpr auto value = R"(
typedef float8 bands_type;
)";
};

constexpr auto make_bands_type(float f) {
    return construct_vector<simulation_bands>(f);
}

//----------------------------------------------------------------------------//

template <size_t bands>
struct alignas(1 << 5) surface {
    using container = detail::cl_vector_constructor_t<float, bands>;
    container absorption;
    container scattering;
};

template <>
struct cl_representation<surface<simulation_bands>> final {
    static constexpr auto value = R"(
typedef struct {
    bands_type absorption;
    bands_type scattering;
} surface;
)";
};

template <size_t bands>
constexpr auto make_surface(float s, float d) {
    return surface<bands>{construct_vector<bands>(s),
                          construct_vector<bands>(d)};
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
    static constexpr auto value = R"(
typedef struct {
    uint surface;
    uint v0;
    uint v1;
    uint v2;
} triangle;
)";
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
    static constexpr auto value = R"(
typedef struct {
    float3 v0;
    float3 v1;
    float3 v2;
} triangle_verts;
)";
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
