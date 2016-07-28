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
    char keep_going;
    char receiver_visible;
} Reflection;

typedef struct {
    VolumeType specular;
    float distance;
} DiffusePathInfo;

typedef struct {
    float3 v0;
    float3 v1;
    float3 v2;
} TriangleVerts;

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
    char intersects;
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
} Microphone;

typedef struct {
    float3 c0;
    float3 c1;
} AABB;

)");
}  // namespace cl_sources
