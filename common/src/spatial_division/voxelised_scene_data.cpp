#include "common/almost_equal.h"
#include "common/azimuth_elevation.h"
#include "common/geo/geometric.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include <random>

voxelised_scene_data::voxelised_scene_data(
        const copyable_scene_data& scene_data,
        size_t octree_depth,
        const geo::box& aabb)
        : scene_data(scene_data)
        , voxels(ndim_tree<3>(
                  octree_depth,
                  [&](auto item, const auto& aabb) {
                      return geo::overlaps(
                              util::padded(aabb,
                                           glm::vec3{0.001}),  // this is a bit
                                                               // greedy - we're
                                                               // sacrificing a
                                                               // some speed
                                                               // for
                                                               // correctness
                              geo::get_triangle_vec3(
                                      scene_data.get_triangles()[item],
                                      scene_data.get_vertices()));
                  },
                  scene_data.compute_triangle_indices(),
                  aabb)) {
    if (aabb == scene_data.get_aabb()) {
        throw std::runtime_error(
                "remember to add some padding to the voxelisation "
                "boundary!");
    }
}

const copyable_scene_data& voxelised_scene_data::get_scene_data() const {
    return scene_data;
}
const voxel_collection<3>& voxelised_scene_data::get_voxels() const {
    return voxels;
}

//----------------------------------------------------------------------------//

std::experimental::optional<intersection> intersects(
        const voxelised_scene_data& voxelised,
        const geo::ray& ray,
        size_t to_ignore) {
    std::experimental::optional<intersection> state;
    traverse(voxelised.get_voxels(),
             ray,
             [&](const geo::ray& ray,
                 const aligned::vector<size_t>& to_test,
                 float min_dist_inside_voxel,
                 float max_dist_inside_voxel) {
                 const auto i = geo::ray_triangle_intersection(
                         ray,
                         to_test,
                         voxelised.get_scene_data().get_triangles(),
                         voxelised.get_scene_data().get_vertices(),
                         to_ignore);
                 if (i && i->inter.t <= max_dist_inside_voxel) {
                     state = i;
                     return true;
                 }
                 return false;
             });
    return state;
}

//	Will return nullopt if a degenerate intersection is detected, otherwise will
//	return the number of (good, wholesome) intersections along the ray
std::experimental::optional<size_t> count_intersections(
        const voxelised_scene_data& voxelised, const geo::ray& ray) {
    size_t count{0};
    bool degenerate{false};
    //	for each voxel along the ray
    traverse(voxelised.get_voxels(),
             ray,
             [&](const geo::ray& ray,
                 const aligned::vector<size_t>& to_test,
                 float min_dist_inside_voxel,
                 float max_dist_inside_voxel) {
                 //	 for each triangle in the voxel
                 for (const auto i : to_test) {
                     //  find any intersection between the triangle and the
                     //  ray
                     const auto intersection = triangle_intersection(
                             voxelised.get_scene_data().get_triangles()[i],
                             voxelised.get_scene_data().get_vertices(),
                             ray);
                     //  if there is an intersection
                     if (intersection) {
                         if (almost_equal(intersection->t, 0.0f, 10)) {
                             //  ray pos is on the boundary, so say it's inside
                             count += 1;
                             return true;
                         }
                         if (is_degenerate(*intersection)) {
                             //  if the intersection is degenerate, set the
                             //  'degenerate' flag to true, and quit
                             //  traversal
                             degenerate = true;
                             return true;
                         }
                         //  if the intersection is inside the current voxel
                         if (min_dist_inside_voxel < intersection->t &&
                             intersection->t <= max_dist_inside_voxel) {
                             //	 increment the intersection counter
                             count += 1;
                         }
                     }
                 }
                 return false;
             });
    if (degenerate) {
        return std::experimental::nullopt;
    }
    return count;
}

namespace {
std::experimental::optional<bool> is_inside(
        const voxelised_scene_data& voxelised, const geo::ray& ray) {
    if (const auto i = count_intersections(voxelised, ray)) {
        return *i % 2;
    }
    return std::experimental::nullopt;
}
}  // namespace

bool inside(const voxelised_scene_data& voxelised, const glm::vec3& pt) {
    std::default_random_engine engine{std::random_device()()};
    for (;;) {
        const direction_rng rng(engine);
        const geo::ray ray{pt, sphere_point(rng.get_z(), rng.get_theta())};
        if (const auto i = is_inside(voxelised, ray)) {
            return *i;
        }
    }
}
