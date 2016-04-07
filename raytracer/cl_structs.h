#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "app_config.h"
#include "scene_data.h"

#define NUM_IMAGE_SOURCE 10
//  These definitions MUST be kept up-to-date with the defs in the cl file.
//  It might make sense to nest them inside the Scene because I don't think
//  other classes will need the same data formats.

/// An impulse contains a volume, a time in seconds, and the direction from
/// which it came (useful for attenuation/hrtf stuff).
typedef struct {
    VolumeType volume;
    cl_float3 position;
    cl_float time;
} __attribute__((aligned(8))) Impulse;

typedef struct {
    VolumeType volume;
    cl_float time;
} __attribute__((aligned(8))) AttenuatedImpulse;

/// Each speaker has a (normalized-unit) direction, and a coefficient in the
/// range 0-1 which describes its polar pattern from omni to bidirectional.
typedef struct {
    cl_float3 direction;
    cl_float coefficient;
} __attribute__((aligned(8))) Speaker;

typedef struct {
    cl_float3 position;
    cl_float3 direction;
} __attribute__((aligned(8))) Ray;

typedef struct {
    cl_float3 v0;
    cl_float3 v1;
    cl_float3 v2;
} __attribute__((aligned(8))) TriangleVerts;

typedef struct {
    TriangleVerts prev_primitives[NUM_IMAGE_SOURCE - 1];
    Ray ray;
    VolumeType volume;
    cl_float3 mic_reflection;
    cl_float distance;
    cl_uint cont;
} __attribute__((aligned(8))) RayInfo;

typedef struct {
    cl_float3 c0;
    cl_float3 c1;
} __attribute__((aligned(8))) AABB;
