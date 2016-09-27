#pragma once

#include "raytracer/image_source/postprocessors.h"
#include "raytracer/postprocess.h"

#include "common/callback_accumulator.h"
#include "common/geo/box.h"

/// For a cuboid with perfectly reflective walls, the image source solution is
/// exact and fast to calculate.
/// Here we implement a fast solver for image sources in a cuboid.

namespace raytracer {
namespace image_source {

template <typename T>
constexpr T power(T t, size_t exponent) {
    return !exponent ? 1 : t * power(t, exponent - 1);
}

constexpr auto width_for_shell(size_t shell) { return shell * 2 + 1; }
constexpr auto num_images(size_t shell) {
    return power(width_for_shell(shell), 3);
}

inline auto image_position(const geo::box& box,
                           const glm::vec3& source,
                           const glm::ivec3& image_index) {
    return glm::mix(source,
                    centre(box) * 2.0f - source,
                    glm::equal(image_index % 2, glm::ivec3{1})) +
           dimensions(box) * glm::vec3{image_index};
}


template <typename Callback>
void traverse_images(const geo::box& box,
                     const glm::vec3& source,
                     size_t shell,
                     const Callback& callback) {
    const auto width{width_for_shell(shell)};
    for (auto i{0ul}; i != width; ++i) {
        const auto x{i - shell};
        for (auto j{0ul}; j != width; ++j) {
            const auto y{j - shell};
            for (auto k{0ul}; k != width; ++k) {
                const auto z{k - shell};

                const glm::ivec3 image_index{x, y, z};
                //  Find image position.
                const auto pos{image_position(box, source, image_index)};
                //  Find intersection angles.
                const auto shell_dim{glm::abs(image_index)};
                const auto reflections{shell_dim.x + shell_dim.y + shell_dim.z};
                if (reflections < shell) {
                    //  Find angles with each of the walls.
                    const auto direction{glm::normalize(pos - source)};
                    const auto cos_x_angle{
                            glm::dot(direction, glm::vec3{1, 0, 0})};
                    const auto cos_y_angle{
                            glm::dot(direction, glm::vec3{0, 1, 0})};
                    const auto cos_z_angle{
                            glm::dot(direction, glm::vec3{0, 0, 1})};

                    //  Add the right number of reflections with each angle.
                    //  This is fine supposing the reflection operation is
                    //  assosiative.
                    aligned::vector<reflection_metadata> intersections{};
                    intersections.reserve(reflections);
                    for (auto c{0ul}; c != shell_dim.x; ++c) {
                        intersections.emplace_back(
                                reflection_metadata{0, cos_x_angle});
                    }
                    for (auto c{0ul}; c != shell_dim.y; ++c) {
                        intersections.emplace_back(
                                reflection_metadata{0, cos_y_angle});
                    }
                    for (auto c{0ul}; c != shell_dim.z; ++c) {
                        intersections.emplace_back(
                                reflection_metadata{0, cos_z_angle});
                    }

                    //  Call the callback.
                    callback(pos, intersections.begin(), intersections.end());
                }
            }
        }
    }
}

template <typename Callback, typename Surface>
auto find_impulses(const geo::box& box,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const Surface& surface,
        size_t shells) {
    aligned::vector<Surface> surfaces{surface};
    callback_accumulator<Callback> callback{receiver, surfaces, false};
    traverse_images(
            box, source, shells, [&](const auto& img, auto begin, auto end) {
                callback(img, begin, end);
            });
    return callback.get_output();
}

template <typename Callback, typename Surface>
auto find_impulses(const geo::box& box,
                   const glm::vec3& source,
                   const glm::vec3& receiver,
                   const Surface& surface) {
    return find_impulses<Callback>(
            box,
            source,
            receiver,
            surface,
            compute_optimum_reflection_number(min_absorption(surface)));
}

}  // namespace image_source
}  // namespace raytracer
