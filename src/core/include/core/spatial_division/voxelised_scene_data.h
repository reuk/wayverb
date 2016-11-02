#pragma once

#include "core/azimuth_elevation.h"
#include "core/scene_data.h"
#include "core/spatial_division/voxel_collection.h"

#include <random>

namespace core {

template <typename Vertex, typename Surface>
class voxelised_scene_data final {
public:
    //  invariant:
    //  The 'voxels' structure holds references/indexes to valid triangles in
    //  the 'scene_data' structure.

    using scene_data = generic_scene_data<Vertex, Surface>;

    voxelised_scene_data(scene_data scene,
                         size_t octree_depth,
                         const geo::box& aabb)
            : scene_{std::move(scene)}
            , voxels_{ndim_tree<3>{
                      octree_depth,
                      [this](auto item, const auto& aabb) {
                          // This is a bit greedy - we're sacrificing some speed
                          // in the name of correctness.
                          return geo::overlaps(
                                  padded(aabb, glm::vec3{0.001}),
                                  geo::get_triangle_vec3(
                                          scene_.get_triangles()[item],
                                          scene_.get_vertices()));
                      },
                      compute_triangle_indices(scene_),
                      aabb}} {
        if (aabb == geo::get_aabb(scene_)) {
            throw std::runtime_error{
                    "remember to add some padding to the voxelisation "
                    "boundary!"};
        }
    }

    const scene_data& get_scene_data() const { return scene_; }
    const voxel_collection<3>& get_voxels() const { return voxels_; }

    //  We can allow modifying surfaces without violating the invariant.
    template <typename It>
    void set_surfaces(It begin, It end) {
        scene_.set_surfaces(begin, end);
    }
    void set_surfaces(const Surface& surface) { scene_.set_surfaces(surface); }

private:
    scene_data scene_;
    voxel_collection<3> voxels_;
};

template <typename Vertex, typename Surface, typename T>
auto make_voxelised_scene_data(generic_scene_data<Vertex, Surface> scene,
                               size_t octree_depth,
                               const util::range<T>& aabb) {
    return voxelised_scene_data<Vertex, Surface>{
            std::move(scene), octree_depth, aabb};
}

template <typename Vertex, typename Surface, typename Pad>
auto make_voxelised_scene_data(generic_scene_data<Vertex, Surface> scene,
                               size_t octree_depth,
                               Pad padding) {
    const auto aabb = padded(geo::get_aabb(scene), glm::vec3{padding});
    return make_voxelised_scene_data(std::move(scene), octree_depth, aabb);
}

////////////////////////////////////////////////////////////////////////////////

template <typename Vertex, typename Surface>
std::experimental::optional<intersection> intersects(
        const voxelised_scene_data<Vertex, Surface>& voxelised,
        const geo::ray& ray,
        size_t to_ignore = ~size_t{0}) {
    std::experimental::optional<intersection> state;
    traverse(voxelised.get_voxels(),
             ray,
             [&](const geo::ray& ray,
                 const voxel& to_test,
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

template <typename Vertex, typename Surface>
std::experimental::optional<size_t> count_intersections(
        const voxelised_scene_data<Vertex, Surface>& voxelised,
        const geo::ray& ray) {
    size_t count{0};
    bool degenerate{false};
    //	for each voxel along the ray
    traverse(voxelised.get_voxels(),
             ray,
             [&](const geo::ray& ray,
                 const voxel& to_test,
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
template <typename Vertex, typename Surface>
std::experimental::optional<bool> is_inside(
        const voxelised_scene_data<Vertex, Surface>& voxelised,
        const geo::ray& ray) {
    if (const auto i = count_intersections(voxelised, ray)) {
        return *i % 2;
    }
    return std::experimental::nullopt;
}
}  // namespace

template <typename Vertex, typename Surface>
bool inside(const voxelised_scene_data<Vertex, Surface>& voxelised,
            const glm::vec3& pt) {
    std::default_random_engine engine{std::random_device{}()};
    for (;;) {
        const direction_rng rng{engine};
        const geo::ray ray{pt, sphere_point(rng.get_z(), rng.get_theta())};
        if (const auto i{is_inside(voxelised, ray)}) {
            return *i;
        }
    }
}

}  // namespace core
