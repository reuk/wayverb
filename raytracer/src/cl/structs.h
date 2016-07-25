#pragma once

//  please only include in .cpp files

#include <string>

namespace cl_sources {
const std::string structs(R"(

typedef float8 VolumeType;

typedef struct {
    float3 position;
    float3 direction;
} Ray;

typedef struct {
    float3 position;
    float3 direction;
    ulong triangle;
} Reflection;

typedef struct {
    float3 v0;
    float3 v1;
    float3 v2;
} TriangleVerts;

typedef struct {
    Ray ray;
    VolumeType volume;
    float3 image;
    float distance;
    bool keep_going;
} RayInfo;

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
    ulong primitive;
    float distance;
    bool intersects;
} Intersection;

typedef struct {
    VolumeType volume;
    float3 position;
    float time;
} Impulse;

typedef struct {
    VolumeType volume;
    float time;
} AttenuatedImpulse;

typedef struct {
    float3 direction;
    float coefficient;
} Speaker;

typedef struct {
    float3 c0;
    float3 c1;
} AABB;

)");
}  // namespace cl_sources
