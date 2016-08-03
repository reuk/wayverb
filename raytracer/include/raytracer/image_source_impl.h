#pragma once

#include "raytracer/cl_structs.h"

#include "common/aligned/set.h"
#include "common/aligned/vector.h"
#include "common/geometric.h"
#include "common/scene_data.h"
#include "common/triangle_vec.h"
#include "common/voxel_collection.h"

#include "glm/glm.hpp"

#include <experimental/optional>

namespace raytracer {

//  ray paths should be ordered by [ray][depth]
template <typename T>
aligned::set<aligned::vector<cl_ulong>> compute_unique_paths(
        aligned::vector<aligned::vector<T>>&& path) {
    aligned::set<aligned::vector<cl_ulong>> ret;

    //  for each ray
    for (auto j = 0; j != path.size(); ++j) {
        //  get all ray path combinations
        for (auto k = 0; k < path[j].size(); ++k) {
            if (path[j][k].visible) {
                aligned::vector<cl_ulong> surfaces;
                surfaces.reserve(k);
                std::transform(path[j].begin(),
                               path[j].begin() + k + 1,
                               std::back_inserter(surfaces),
                               [](const auto& i) { return i.index; });
                //  add the path to the return set
                ret.insert(surfaces);
            }
        }
    }

    return ret;
}

aligned::vector<TriangleVec3> compute_original_triangles(
        const aligned::vector<cl_ulong>& triangles,
        const CopyableSceneData& scene_data);

aligned::vector<TriangleVec3> compute_mirrored_triangles(
        const aligned::vector<TriangleVec3>& original,
        const CopyableSceneData& scene_data);

std::experimental::optional<aligned::vector<float>>
compute_intersection_distances(const aligned::vector<TriangleVec3>& mirrored,
                               const geo::Ray& ray);

aligned::vector<glm::vec3> compute_intersection_points(
        const aligned::vector<float>& distances, const geo::Ray& ray);

aligned::vector<glm::vec3> compute_unmirrored_points(
        const aligned::vector<glm::vec3>& points,
        const aligned::vector<TriangleVec3>& original);

geo::Ray construct_ray(const glm::vec3& from, const glm::vec3& to);

glm::vec3 compute_mirrored_point(const aligned::vector<TriangleVec3>& mirrored,
                                 const glm::vec3& original);

float compute_distance(const aligned::vector<glm::vec3>& unmirrored);

VolumeType compute_volume(const CopyableSceneData& scene_data,
                          const aligned::vector<cl_ulong>& triangles);

Impulse compute_ray_path_impulse(const CopyableSceneData& scene_data,
                                 const aligned::vector<cl_ulong>& triangles,
                                 const aligned::vector<glm::vec3>& unmirrored);

std::experimental::optional<Impulse> follow_ray_path(
        const aligned::vector<cl_ulong>& triangles,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const CopyableSceneData& scene_data,
        const VoxelCollection& vox,
        const VoxelCollection::TriangleTraversalCallback& callback);

} //namespace raytracer
