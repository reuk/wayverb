#include "common/spatial_division/voxelised_scene_data.h"
#include "common/almost_equal.h"
#include "common/geo/geometric.h"

voxelised_scene_data::voxelised_scene_data(
        const copyable_scene_data& scene_data,
        size_t octree_depth,
        const geo::box& aabb)
        : scene_data(scene_data)
        , voxels(ndim_tree<3>(
                  octree_depth,
                  [&](auto item, const auto& aabb) {
                      return geo::overlaps(
                              aabb,
                              geo::get_triangle_vec3(
                                      scene_data.get_triangles()[item],
                                      scene_data.get_vertices()));
                  },
                  scene_data.compute_triangle_indices(),
                  aabb)) {}

const copyable_scene_data& voxelised_scene_data::get_scene_data() const {
    return scene_data;
}
const voxel_collection<3>& voxelised_scene_data::get_voxels() const {
    return voxels;
}

//----------------------------------------------------------------------------//

geo::intersection intersects(const voxelised_scene_data& voxelised,
                             const geo::ray& ray) {
    geo::intersection state;
    traverse(voxelised.get_voxels(),
             ray,
             [&](const geo::ray& ray,
                 const aligned::vector<size_t>& to_test,
                 float max_dist_inside_voxel) {
                 const auto i = geo::ray_triangle_intersection(
                         ray,
                         to_test,
                         voxelised.get_scene_data().get_triangles(),
                         voxelised.get_scene_data().get_vertices());
                 if (i && i->distance < max_dist_inside_voxel) {
                     state = i;
                     return true;
                 }
                 return false;
             });
    return state;
}

bool inside(const voxelised_scene_data& voxelised, const glm::vec3& pt) {
    const geo::ray ray{pt, glm::vec3{0, 0, 1}};
    aligned::vector<float> distances;
    traverse(
            voxelised.get_voxels(),
            ray,
            [&](const geo::ray& ray,
                const aligned::vector<size_t>& to_test,
                float max_dist_inside_voxel) {
                for (const auto i : to_test) {
                    const auto intersection = triangle_intersection(
                            voxelised.get_scene_data().get_triangles()[i],
                            voxelised.get_scene_data().get_vertices(),
                            ray);
                    if (intersection && *intersection < max_dist_inside_voxel) {
                        if (proc::find_if(distances, [&intersection](auto i) {
                                return almost_equal(
                                        i, *intersection, size_t{10});
                            }) == distances.end()) {
                            distances.push_back(*intersection);
                        }
                    }
                }
                return false;
            });
    return distances.size() % 2;
}
