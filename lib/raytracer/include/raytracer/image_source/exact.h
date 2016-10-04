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
    const auto mirrored{centre(box) * 2.0f - source};
    const auto selector{glm::equal(glm::abs(image_index) % 2, glm::ivec3{1})};
    return glm::mix(source, mirrored, selector) +
           dimensions(box) * glm::vec3{image_index};
}

template <typename Callback>
void image_at_index(const geo::box& box,
                    const glm::vec3& source,
                    const glm::vec3& receiver,
                    const glm::ivec3& image_index,
                    double max_distance,
                    aligned::vector<reflection_metadata>& scratch,
                    const Callback& callback) {
    const auto pos{image_position(box, source, image_index)};
    if (max_distance < glm::distance(receiver, pos)) {
        return;
    }

    //  Find intersection angles.
    const auto shell_dim{glm::abs(image_index)};
    //  Find angles with each of the walls.
    const auto direction{glm::normalize(pos - receiver)};
    const auto cos_angles{glm::abs(direction)};

    //  Add the right number of reflections with each angle.
    //  This is fine supposing the reflection operation is associative.
    scratch.clear();
    for (auto c{0ul}; c != shell_dim.x; ++c) {
        scratch.emplace_back(reflection_metadata{0, cos_angles.x});
    }
    for (auto c{0ul}; c != shell_dim.y; ++c) {
        scratch.emplace_back(reflection_metadata{0, cos_angles.y});
    }
    for (auto c{0ul}; c != shell_dim.z; ++c) {
        scratch.emplace_back(reflection_metadata{0, cos_angles.z});
    }

    //  Call the callback.
    callback(pos, scratch.begin(), scratch.end());
}

template <typename Callback>
void traverse_images(const geo::box& box,
                     const glm::vec3& source,
                     const glm::vec3& receiver,
                     size_t shells,
                     double max_distance,
                     const Callback& callback) {
    aligned::vector<reflection_metadata> scratch;
    const auto width{width_for_shell(shells)};
    for (auto i{0ul}; i != width; ++i) {
        const auto x{i - shells};
        for (auto j{0ul}; j != width; ++j) {
            const auto y{j - shells};
            for (auto k{0ul}; k != width; ++k) {
                const auto z{k - shells};

                image_at_index(box,
                               source,
                               receiver,
                               glm::ivec3{x, y, z},
                               max_distance,
                               scratch,
                               callback);
            }
        }
    }
}

template <typename Callback, typename Surface>
auto find_impulses(const geo::box& box,
                   const glm::vec3& source,
                   const glm::vec3& receiver,
                   const Surface& surface,
                   double max_distance) {
    const auto dim{dimensions(box)};
    const auto min_dim{std::min({dim.x, dim.y, dim.z})};
    const auto shells{std::ceil(max_distance / min_dim)};

    aligned::vector<Surface> surfaces{surface};
    callback_accumulator<Callback> callback{receiver, surfaces, false};
    traverse_images(box,
                    source,
                    receiver,
                    shells,
                    max_distance,
                    [&](const auto& img, auto begin, auto end) {
                        callback(img, begin, end);
                    });
    return callback.get_output();
}

template <typename Callback, typename Surface>
auto find_all_impulses(const geo::box& box,
                       const glm::vec3& source,
                       const glm::vec3& receiver,
                       const Surface& surface,
                       double speed_of_sound) {
    const auto rt{
            eyring_reverb_time(geo::get_scene_data(box, surface), 0).s[0]};
    const auto max_dist{rt * speed_of_sound};
    // const auto shells{
    //        compute_optimum_reflection_number(min_absorption(surface))};
    return find_impulses<Callback>(box, source, receiver, surface, max_dist);
}

}  // namespace image_source
}  // namespace raytracer
