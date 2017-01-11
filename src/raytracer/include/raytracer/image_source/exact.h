#pragma once

#include "raytracer/image_source/fast_pressure_calculator.h"

#include "core/callback_accumulator.h"
#include "core/geo/box.h"
#include <iostream>

/// \file exact.h
/// For a cuboid with perfectly reflective walls, the image source solution is
/// exact and fast to calculate.
/// Here we implement a fast solver for image sources in a cuboid.

namespace wayverb {
namespace raytracer {
namespace image_source {

//  Combined Wave and Ray Based Room Acoustic Simulations of Small Rooms
//  Marc Aretz, p. 71

glm::vec3 image_source_position(const glm::ivec3& order,
                                const glm::vec3& source,
                                const glm::vec3& dim);

template <typename T>
constexpr T power(T t, size_t p) {
    return p == 0 ? core::unit_constructor_v<T> : t * power(t, p - 1);
}

template <typename T>
auto attenuation_factor(const glm::ivec3& order,
                        const glm::vec3& image_source,
                        const glm::vec3& receiver,
                        const T& impedance) {
    const auto diff = image_source - receiver;
    const auto cos_theta = glm::abs(diff) / glm::length(diff);

    return power(core::average_wall_impedance_to_pressure_reflectance(
                         impedance, cos_theta.x),
                 abs(order.x)) *
           power(core::average_wall_impedance_to_pressure_reflectance(
                         impedance, cos_theta.y),
                 abs(order.y)) *
           power(core::average_wall_impedance_to_pressure_reflectance(
                         impedance, cos_theta.z),
                 abs(order.z));
}

template <typename T>
auto traverse_images(const core::geo::box& box,
                     const glm::vec3& source,
                     const glm::vec3& receiver,
                     double max_distance,
                     const T& impedance) {
    const auto dim = dimensions(box);
    const auto shells = glm::ivec3(glm::ceil(glm::vec3(max_distance) / dim));

    std::cout << "shells: " << shells.x << ", " << shells.y << ", " << shells.z
              << '\n';

    using impulse_t = impulse<::detail::components_v<T>>;

    util::aligned::vector<impulse_t> impulses;
    impulses.reserve(shells.x * shells.y * shells.z * 8);

    for (auto i = -shells.x; i != shells.x + 1; ++i) {
        for (auto j = -shells.y; j != shells.y + 1; ++j) {
            for (auto k = -shells.z; k != shells.z + 1; ++k) {
                const auto order = glm::ivec3{i, j, k};
                const auto pos = box.get_min() +
                                 image_source_position(order, source, dim);
                const auto dist = glm::distance(pos, receiver);

                if (dist < max_distance) {
                    const auto attenuation =
                            attenuation_factor(order, pos, receiver, impedance);

                    impulses.emplace_back(
                            impulse_t{attenuation,
                                      core::to_cl_float3{}(pos),
                                      glm::distance(pos, receiver)});
                }
            }
        }
    }

    return impulses;
}

template <typename Surface>
auto find_impulses(const core::geo::box& box,
                   const glm::vec3& source,
                   const glm::vec3& receiver,
                   const Surface& surface,
                   double max_distance) {
    const auto impedance = core::pressure_reflectance_to_average_wall_impedance(
            core::absorption_to_pressure_reflectance(surface));

    return traverse_images(box, source, receiver, max_distance, impedance);
}

}  // namespace image_source
}  // namespace raytracer
}  // namespace wayverb
