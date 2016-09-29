#pragma once

#include "common/cl/representation.h"
#include "common/cl/scene_structs.h"
#include "common/cl/traits.h"

/// Rays will not intersect with the same surface that was referenced by the
/// previous reflection along a ray.
/// Ensure that the `triangle` field of the initial reflection buffer is set
/// to a number larger than the total number of triangles in the scene
/// (like ~uint{0})

struct alignas(1 << 4) reflection final {
    cl_float3 position;   //  position of the secondary source
    cl_float3 direction;  //  specular direction from the source
    cl_uint triangle;     //  triangle which contains source
    cl_char keep_going;   //  whether or not this is the teriminator for this
                          //  path (like a \0 in a char*)
    cl_char receiver_visible;  //  whether or not the receiver is visible from
                               //  this point
};

template <>
struct cl_representation<reflection> final {
    static constexpr auto value{R"(
typedef struct {
    float3 position;
    float3 direction;
    uint triangle;
    char keep_going;
    char receiver_visible;
} reflection;
)"};
};

constexpr auto to_tuple(const reflection& x) {
    return std::tie(x.position,
                    x.direction,
                    x.triangle,
                    x.keep_going,
                    x.receiver_visible);
}

constexpr bool operator==(const reflection& a, const reflection& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const reflection& a, const reflection& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

struct alignas(1 << 5) diffuse_path_info final {
    volume_type volume;  //  product of previous specular components
    cl_float3 position;  //  because otherwise we won't be able to
                         //  calculate a new distance
    cl_float distance;   //  total distance travelled
};

template <>
struct cl_representation<diffuse_path_info> final {
    static constexpr auto value{R"(
typedef struct {
    volume_type volume;
    float3 position;
    float distance;
} diffuse_path_info;
)"};
};

constexpr auto to_tuple(const diffuse_path_info& x) {
    return std::tie(x.volume, x.position, x.distance);
}

constexpr bool operator==(const diffuse_path_info& a,
                          const diffuse_path_info& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const diffuse_path_info& a,
                          const diffuse_path_info& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

/// An impulse contains a volume, a time in seconds, and the direction from
/// which it came (useful for attenuation/hrtf stuff).
template <size_t channels = 8>
struct alignas(1 << 5) impulse final {
    detail::cl_vector_constructor_t<float, channels>
            volume;      //  actual per-band volume of the impulse
    cl_float3 position;  //  position of the secondary source (used for
                         //  attenuation)
    cl_float distance;   //  distance that the carrier ray has travelled
};

template <>
struct cl_representation<impulse<8>> final {
    static constexpr auto value{R"(
typedef struct {
    volume_type volume;
    float3 position;
    float distance;
} impulse;
)"};
};

template <size_t channels>
constexpr auto to_tuple(const impulse<channels>& x) {
    return std::tie(x.volume, x.position, x.distance);
}

template <size_t channels>
constexpr bool operator==(const impulse<channels>& a,
                          const impulse<channels>& b) {
    return to_tuple(a) == to_tuple(b);
}

template <size_t channels>
constexpr bool operator!=(const impulse<channels>& a,
                          const impulse<channels>& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

template <size_t channels = 8>
struct alignas(1 << 5) attenuated_impulse final {
    detail::cl_vector_constructor_t<float, channels> volume;
    cl_float distance;
};

template <>
struct cl_representation<attenuated_impulse<8>> final {
    static constexpr auto value{R"(
typedef struct {
    volume_type volume;
    float distance;
} attenuated_impulse;
)"};
};

template <size_t channels>
constexpr auto to_tuple(const attenuated_impulse<channels>& x) {
    return std::tie(x.volume, x.distance);
}

template <size_t channels>
constexpr bool operator==(const attenuated_impulse<channels>& a,
                          const attenuated_impulse<channels>& b) {
    return to_tuple(a) == to_tuple(b);
}

template <size_t channels>
constexpr bool operator!=(const attenuated_impulse<channels>& a,
                          const attenuated_impulse<channels>& b) {
    return !(a == b);
}
