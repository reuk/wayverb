#pragma once

#include "common/cl_include.h"
#include "common/config.h"
#include "common/scene_data.h"

constexpr VolumeType air_coefficient{{
        0.001f * -0.1f,
        0.001f * -0.2f,
        0.001f * -0.5f,
        0.001f * -1.1f,
        0.001f * -2.7f,
        0.001f * -9.4f,
        0.001f * -29.0f,
        0.001f * -60.0f,
}};

/// An impulse contains a volume, a time in seconds, and the direction from
/// which it came (useful for attenuation/hrtf stuff).
struct alignas(1 << 5) Impulse final {
    VolumeType volume;   //  actual per-band volume of the impulse
    cl_float3 position;  //  position of the secondary source (used for
                         //  attenuation)
    cl_float time;       //  time that the impulse is received
};

constexpr auto to_tuple(const Impulse& x) {
    return std::tie(x.volume, x.position, x.time);
}

constexpr bool operator==(const Impulse& a, const Impulse& b) {
    return to_tuple(a) == to_tuple(b);
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

constexpr auto to_tuple(const Reflection& x) {
    return std::tie(x.position,
                    x.direction,
                    x.triangle,
                    x.keep_going,
                    x.receiver_visible);
}

constexpr bool operator==(const Reflection& a, const Reflection& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const Reflection& a, const Reflection& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

struct alignas(1 << 5) DiffusePathInfo final {
    VolumeType volume;    //  product of previous specular components
    cl_float3 position;   //  because otherwise we won't be able to
                          //  calculate a new distance
    cl_float distance;    //  total distance travelled
};

constexpr auto to_tuple(const DiffusePathInfo& x) {
    return std::tie(x.volume, x.position, x.distance);
}

constexpr bool operator==(const DiffusePathInfo& a, const DiffusePathInfo& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const DiffusePathInfo& a, const DiffusePathInfo& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

struct alignas(1 << 5) AttenuatedImpulse final {
    VolumeType volume;
    cl_float time;
};

constexpr auto to_tuple(const AttenuatedImpulse& x) {
    return std::tie(x.volume, x.time);
}

constexpr bool operator==(const AttenuatedImpulse& a,
                          const AttenuatedImpulse& b) {
    return to_tuple(a) == to_tuple(b);
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

constexpr auto to_tuple(const Microphone& x) {
    return std::tie(x.direction, x.coefficient);
}

constexpr bool operator==(const Microphone& a, const Microphone& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const Microphone& a, const Microphone& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

struct alignas(1 << 4) Ray final {
    cl_float3 position;
    cl_float3 direction;
};

constexpr auto to_tuple(const Ray& x) {
    return std::tie(x.position, x.direction);
}

constexpr bool operator==(const Ray& a, const Ray& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const Ray& a, const Ray& b) { return !(a == b); }

//----------------------------------------------------------------------------//

struct alignas(1 << 4) AABB final {
    cl_float3 c0;
    cl_float3 c1;
};

constexpr auto to_tuple(const AABB& x) { return std::tie(x.c0, x.c1); }

constexpr bool operator==(const AABB& a, const AABB& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const AABB& a, const AABB& b) { return !(a == b); }
