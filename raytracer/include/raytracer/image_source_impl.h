#pragma once

#include "raytracer/cl_structs.h"

#include "common/aligned/set.h"
#include "common/aligned/vector.h"
#include "common/geo/geometric.h"
#include "common/geo/triangle_vec.h"
#include "common/map.h"
#include "common/scene_data.h"
#include "common/voxel_collection.h"

#include "glm/glm.hpp"

#include <experimental/optional>

namespace raytracer {

/// A ray_path is an ordered collection of items which have a 'visible' field
/// (denoting whether the intersection is visible from the receiver) and an
/// 'index' field (denoting which primitive has been intersected).
template <typename T>
using ray_path = aligned::vector<T>;

/// An index_path just contains the orders of intersected primitives along a ray
/// path.
using index_path = aligned::vector<cl_ulong>;

/// Given the paths of several rays, find all unique index paths which might
/// contribute an image-source impulse.
/// Ray paths should be ordered by [ray][depth].
template <typename T>
aligned::set<index_path> compute_unique_paths(
        aligned::vector<ray_path<T>>&& path) {
    aligned::set<index_path> ret;

    //  for each ray
    for (auto j = 0; j != path.size(); ++j) {
        //  get all ray path combinations
        for (auto k = 0; k < path[j].size(); ++k) {
            if (path[j][k].visible) {
                //  add the path to the return set
                ret.insert(
                        map_to_vector(path[j].begin(),
                                      path[j].begin() + k + 1,
                                      [](const auto& i) { return i.index; }));
            }
        }
    }

    return ret;
}

aligned::vector<geo::triangle_vec3> compute_original_triangles(
        const aligned::vector<cl_ulong>& triangles,
        const copyable_scene_data& scene_data);

aligned::vector<geo::triangle_vec3> compute_mirrored_triangles(
        const aligned::vector<geo::triangle_vec3>& original);

std::experimental::optional<aligned::vector<float>>
compute_intersection_distances(
        const aligned::vector<geo::triangle_vec3>& mirrored,
        const geo::ray& ray);

aligned::vector<glm::vec3> compute_intersection_points(
        const aligned::vector<float>& distances, const geo::ray& ray);

aligned::vector<glm::vec3> compute_unmirrored_points(
        const aligned::vector<glm::vec3>& points,
        const aligned::vector<geo::triangle_vec3>& original);

geo::ray construct_ray(const glm::vec3& from, const glm::vec3& to);

glm::vec3 compute_mirrored_point(
        const aligned::vector<geo::triangle_vec3>& mirrored,
        const glm::vec3& original);

float compute_distance(const aligned::vector<glm::vec3>& unmirrored);

volume_type compute_volume(const copyable_scene_data& scene_data,
                           const aligned::vector<cl_ulong>& triangles);

impulse compute_ray_path_impulse(const copyable_scene_data& scene_data,
                                 const aligned::vector<cl_ulong>& triangles,
                                 const aligned::vector<glm::vec3>& unmirrored);

std::experimental::optional<impulse> follow_ray_path(
        const aligned::vector<cl_ulong>& triangles,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const copyable_scene_data& scene_data,
        const voxel_collection<3>& vox,
        const triangle_traversal_callback& callback);

}  // namespace raytracer
