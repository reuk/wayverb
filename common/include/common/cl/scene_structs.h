#pragma once

//  please only include in .cpp files

#include "common/cl_include.h"
#include <string>

namespace cl_sources {
const std::string scene_structs(R"(

#ifndef SCENE_STRUCTS_HEADER__
#define SCENE_STRUCTS_HEADER__

typedef float8 VolumeType;

typedef struct {
    VolumeType specular;
    VolumeType diffuse;
} Surface;

typedef struct {
    ulong surface;
    ulong v0;
    ulong v1;
    ulong v2;
} Triangle;

typedef struct {
    float3 v0;
    float3 v1;
    float3 v2;
} TriangleVerts;

#endif

)");
}  // namespace cl_sources

using volume_type = cl_float8;

struct alignas(1 << 5) surface {
    volume_type specular{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
    volume_type diffuse{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
};

struct alignas(1 << 3) triangle final {
    cl_ulong surface;
    cl_ulong v0;
    cl_ulong v1;
    cl_ulong v2;
};
