#pragma once

#include "common/cl_include.h"
#include "common/config.h"
#include "common/scene_data.h"

/// An impulse contains a volume, a time in seconds, and the direction from
/// which it came (useful for attenuation/hrtf stuff).
struct alignas(1 << 5) Impulse final {
    VolumeType volume;   //  actual per-band volume of the impulse
    cl_float3 position;  //  position of the secondary source (used for
                         //  attenuation)
    cl_float time;       //  time that the impulse is received
};

constexpr bool operator==(const Impulse& a, const Impulse& b) {
    return std::tie(a.volume, a.position, a.time) ==
           std::tie(b.volume, b.position, b.time);
}

constexpr bool operator!=(const Impulse& a, const Impulse& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

struct alignas(1 << 4) Reflection final {
    cl_float3 position;   //  position of the secondary source
    cl_float3 direction;  //  specular direction from the source
    cl_ulong triangle;    //  triangle which contains source
    cl_char keep_going;   //  whether or not this is the teriminator for this
                          //  path (like a \0 in a char*)
    cl_char receiver_visible;  //  whether or not the receiver is visible from
                               //  this point
};

constexpr bool operator==(const Reflection& a, const Reflection& b) {
    return std::tie(a.position,
                    a.direction,
                    a.triangle,
                    a.keep_going,
                    a.receiver_visible) == std::tie(b.position,
                                                    b.direction,
                                                    b.triangle,
                                                    b.keep_going,
                                                    b.receiver_visible);
}

constexpr bool operator!=(const Reflection& a, const Reflection& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

struct alignas(1 << 5) DiffusePathInfo final {
    VolumeType
            specular;   //  product of specular components of previous surfaces
    cl_float distance;  //  total distance travelled
};

constexpr bool operator==(const DiffusePathInfo& a, const DiffusePathInfo& b) {
    return std::tie(a.specular, a.distance) == std::tie(b.specular, b.distance);
}

constexpr bool operator!=(const DiffusePathInfo& a, const DiffusePathInfo& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

struct alignas(1 << 5) AttenuatedImpulse final {
    VolumeType volume;
    cl_float time;
};

constexpr bool operator==(const AttenuatedImpulse& a,
                          const AttenuatedImpulse& b) {
    return std::tie(a.volume, a.time) == std::tie(b.volume, b.time);
}

constexpr bool operator!=(const AttenuatedImpulse& a,
                          const AttenuatedImpulse& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

/// Each speaker has a (normalized-unit) direction, and a coefficient in the
/// range 0-1 which describes its polar pattern from omni to bidirectional.
struct alignas(1 << 4) Microphone final {
    cl_float3 direction;
    cl_float coefficient;
};

constexpr bool operator==(const Microphone& a, const Microphone& b) {
    return std::tie(a.direction, a.coefficient) ==
           std::tie(b.direction, b.coefficient);
}

constexpr bool operator!=(const Microphone& a, const Microphone& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

struct alignas(1 << 4) Ray final {
    cl_float3 position;
    cl_float3 direction;
};

constexpr bool operator==(const Ray& a, const Ray& b) {
    return std::tie(a.position, a.direction) ==
           std::tie(b.position, b.direction);
}

constexpr bool operator!=(const Ray& a, const Ray& b) { return !(a == b); }

//----------------------------------------------------------------------------//

struct alignas(1 << 4) AABB final {
    cl_float3 c0;
    cl_float3 c1;
};

constexpr bool operator==(const AABB& a, const AABB& b) {
    return std::tie(a.c0, a.c1) == std::tie(b.c0, b.c1);
}

constexpr bool operator!=(const AABB& a, const AABB& b) { return !(a == b); }
