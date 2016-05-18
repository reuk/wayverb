#pragma once

#include "common/cl_include.h"
#include "common/config.h"
#include "common/scene_data.h"

#define NUM_IMAGE_SOURCE 10
//  These definitions MUST be kept up-to-date with the defs in the cl file.
//  It might make sense to nest them inside the Scene because I don't think
//  other classes will need the same data formats.

/// An impulse contains a volume, a time in seconds, and the direction from
/// which it came (useful for attenuation/hrtf stuff).
struct __attribute__((aligned(8))) Impulse final {
    VolumeType volume;
    cl_float3 position;
    cl_float time;
};

struct __attribute__((aligned(8))) AttenuatedImpulse final {
    VolumeType volume;
    cl_float time;
};

/// Each speaker has a (normalized-unit) direction, and a coefficient in the
/// range 0-1 which describes its polar pattern from omni to bidirectional.
struct __attribute__((aligned(8))) Speaker final {
    cl_float3 direction;
    cl_float coefficient;
};

struct __attribute__((aligned(8))) Ray final {
    cl_float3 position;
    cl_float3 direction;
};

struct __attribute__((aligned(8))) TriangleVerts final {
    cl_float3 v0;
    cl_float3 v1;
    cl_float3 v2;
};

struct __attribute__((aligned(8))) RayInfo final {
    TriangleVerts prev_primitives[NUM_IMAGE_SOURCE - 1];
    Ray ray;
    VolumeType volume;
    cl_float3 mic_reflection;
    cl_float distance;
    cl_uint cont;
};

struct __attribute__((aligned(8))) AABB final {
    cl_float3 c0;
    cl_float3 c1;
};
