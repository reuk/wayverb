#pragma once

#include "raytracer/construct_impulse.h"
#include "raytracer/image_source.h"
#include "raytracer/multitree.h"

#include "common/aligned/vector.h"
#include "common/cl/include.h"
#include "common/geo/triangle_vec.h"
#include "common/spatial_division/voxelised_scene_data.h"

namespace raytracer {

/// Each item in the tree references an intersected triangle, which may or
/// may not be visible from the receiver.
using path_element = image_source_finder::item;

/// Need this because we'll be storing path_elements in a set.
constexpr bool operator<(const path_element& a, const path_element& b) {
    return a.index < b.index;
}

//----------------------------------------------------------------------------//

multitree<path_element>::branches_type construct_image_source_tree(
        const aligned::vector<aligned::vector<path_element>>& paths);

//----------------------------------------------------------------------------//

/// The trick here is that the callback can be a stateful object...
template <typename T, typename Callback>
void traverse_multitree(const multitree<T>& tree, const Callback& callback) {
    const auto next{callback(tree.item)};
    for (const auto& i : tree.branches) {
        traverse_multitree(i, next);
    }
}

//----------------------------------------------------------------------------//

template <typename It>
geo::triangle_vec3 compute_mirrored_triangle(It begin,
                                             It end,
                                             geo::triangle_vec3 original) {
    for (; begin != end; ++begin) {
        original = geo::mirror(original, *begin);
    }
    return original;
}

geo::ray construct_ray(const glm::vec3& from, const glm::vec3& to);

template <typename It>
std::experimental::optional<aligned::vector<float>>
compute_intersection_distances(It begin, It end, const geo::ray& ray) {
    aligned::vector<float> ret{};

    for (; begin != end; ++begin) {
        const auto intersection{geo::triangle_intersection(*begin, ray)};
        if (!intersection) {
            return std::experimental::nullopt;
        }
        ret.push_back(intersection->t);
    }

    return ret;
}

aligned::vector<glm::vec3> compute_intersection_points(
        const aligned::vector<float>& distances, const geo::ray& ray);

template <typename It>
aligned::vector<glm::vec3> compute_unmirrored_points(
        It begin, It end, const aligned::vector<glm::vec3>& points) {
    aligned::vector<glm::vec3> ret{points};

    for (auto i{0u}; begin != end; ++i, ++begin) {
        const auto triangle{*begin};
        for (auto j{i + 1}; j != points.size(); ++j) {
            ret[j] = geo::mirror(ret[j], triangle);
        }
    }

    return ret;
}

float compute_distance(const glm::vec3& source,
                       const aligned::vector<glm::vec3>& unmirrored,
                       const glm::vec3& receiver);

template <typename It>
volume_type compute_volume(It begin, It end, const scene_data& scene_data) {
    return std::accumulate(
            begin,
            end,
            volume_type{{1, 1, 1, 1, 1, 1, 1, 1}},
            [&](const auto& volume, const auto& i) {
                const auto scene_triangle{scene_data.get_triangles()[i]};
                const auto surface{
                        scene_data.get_surfaces()[scene_triangle.surface]};
                return volume * surface.specular;
            });
}

template <typename It>
impulse compute_ray_path_impulse(It begin,
                                 It end,
                                 const scene_data& scene_data,
                                 const glm::vec3& source,
                                 const aligned::vector<glm::vec3>& unmirrored,
                                 const glm::vec3& receiver,
                                 double speed_of_sound) {
    return construct_impulse(compute_volume(begin, end, scene_data),
                             unmirrored.back(),
                             compute_distance(source, unmirrored, receiver),
                             speed_of_sound);
}

//----------------------------------------------------------------------------//

struct constants final {
    const glm::vec3& source;
    const glm::vec3& receiver;
    const voxelised_scene_data& voxelised;
    const double speed_of_sound;
};

struct state final {
    cl_ulong index;
    geo::triangle_vec3 original_triangle;
    geo::triangle_vec3 mirrored_triangle;
    glm::vec3 mirrored_receiver;
};

template <typename It>
class image_source_traversal_callback final {
public:
    image_source_traversal_callback(const constants& constants,
                                    aligned::vector<state>& state,
                                    It output_iterator,
                                    const path_element& p);

    ~image_source_traversal_callback() noexcept;

    image_source_traversal_callback operator()(const path_element& p) const;

private:
    std::experimental::optional<impulse> compute_impulse() const;

    const constants& constants_;
    aligned::vector<state>& state_;
    It output_iterator_;
};

//----------------------------------------------------------------------------//

aligned::vector<impulse> compute_impulses(
        const aligned::vector<aligned::vector<path_element>>& paths,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        double speed_of_sound);

}  // namespace raytracer
