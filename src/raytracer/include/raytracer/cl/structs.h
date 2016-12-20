#pragma once

#include "raytracer/cl/reflection.h"

#include "core/cl/representation.h"
#include "core/cl/scene_structs.h"
#include "core/cl/traits.h"

namespace wayverb {
namespace raytracer {

struct alignas(1 << 5) stochastic_path_info final {
    core::bands_type volume;  //  product of previous specular components
    cl_float3 position;       //  because otherwise we won't be able to
                              //  calculate a new distance
    cl_float distance;        //  total distance travelled
};

constexpr auto to_tuple(const stochastic_path_info& x) {
    return std::tie(x.volume, x.position, x.distance);
}

constexpr bool operator==(const stochastic_path_info& a,
                          const stochastic_path_info& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const stochastic_path_info& a,
                          const stochastic_path_info& b) {
    return !(a == b);
}

////////////////////////////////////////////////////////////////////////////////

/// An impulse contains a volume, a time in seconds, and the direction from
/// which it came (useful for attenuation/hrtf stuff).
template <size_t channels>
struct alignas(1 << 5) impulse final {
    detail::cl_vector_constructor_t<float, channels>
            volume;      //  actual per-band volume of the impulse
    cl_float3 position;  //  position of the secondary source (used for
                         //  attenuation)
    cl_float distance;   //  distance that the carrier ray has travelled
};

template <typename T>
constexpr auto make_impulse(T volume, cl_float3 position, cl_float distance) {
    return impulse<detail::components_v<T>>{volume, position, distance};
}

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

////////////////////////////////////////////////////////////////////////////////

template <size_t channels>
struct alignas(1 << 5) attenuated_impulse final {
    detail::cl_vector_constructor_t<float, channels> volume;
    cl_float distance;
};

template <typename T>
constexpr auto make_attenuated_impulse(T volume, cl_float distance) {
    return attenuated_impulse<detail::components_v<T>>{volume, distance};
}

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

}  // namespace raytracer

template <>
struct core::cl_representation<raytracer::reflection> final {
    static constexpr auto value = R"(
typedef struct {
    float3 position;
    uint triangle;
    char keep_going;
    char receiver_visible;
} reflection;
)";
};

template <>
struct core::cl_representation<raytracer::stochastic_path_info> final {
    static constexpr auto value = R"(
typedef struct {
    bands_type volume;
    float3 position;
    float distance;
} stochastic_path_info;
)";
};

template <>
struct core::cl_representation<raytracer::impulse<8>> final {
    static constexpr auto value = R"(
typedef struct {
    bands_type volume;
    float3 position;
    float distance;
} impulse;
)";
};

template <>
struct core::cl_representation<raytracer::attenuated_impulse<8>> final {
    static constexpr auto value = R"(
typedef struct {
    bands_type volume;
    float distance;
} attenuated_impulse;
)";
};

}  // namespace wayverb
