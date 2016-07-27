#pragma once

#include "common/cl_include.h"
#include "common/config.h"
#include "common/scene_data.h"

//  These definitions MUST be kept up-to-date with the defs in the cl file.
//  It might make sense to nest them inside the Scene because I don't think
//  other classes will need the same data formats.

/// An impulse contains a volume, a time in seconds, and the direction from
/// which it came (useful for attenuation/hrtf stuff).
struct alignas(1 << 5) Impulse final {
    VolumeType volume;
    cl_float3 position;
    cl_float time;
};

struct alignas(1 << 4) Reflection final {
    cl_float3 position;
    cl_float3 direction;
    cl_ulong triangle;
    cl_char keep_going;
    cl_char receiver_visible;
};

constexpr bool operator==(const Impulse& a, const Impulse& b) {
    return std::tie(a.volume, a.position, a.time) ==
           std::tie(b.volume, b.position, b.time);
}

constexpr bool operator!=(const Impulse& a, const Impulse& b) {
    return !(a == b);
}

struct alignas(1 << 5) AttenuatedImpulse final {
    VolumeType volume;
    cl_float time;
};

/// Each speaker has a (normalized-unit) direction, and a coefficient in the
/// range 0-1 which describes its polar pattern from omni to bidirectional.
struct alignas(1 << 4) Microphone final {
    cl_float3 direction;
    cl_float coefficient;
};

struct alignas(1 << 4) Ray final {
    cl_float3 position;
    cl_float3 direction;
};

struct alignas(1 << 4) AABB final {
    cl_float3 c0;
    cl_float3 c1;
};
