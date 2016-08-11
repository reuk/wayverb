#include "common/voxelised_scene_data.h"
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
    //  TODO test, might not modify state properly
    class callback final {
    public:
        callback(const copyable_scene_data& scene)
                : scene(scene) {}

        bool operator()(const geo::ray& ray,
                        const aligned::vector<size_t>& to_test,
                        float max_dist_inside_voxel) {
            const auto i = geo::ray_triangle_intersection(
                    ray, to_test, scene.get_triangles(), scene.get_vertices());
            if (i && i->distance < max_dist_inside_voxel) {
                state = i;
                return true;
            }
            return false;
        }

        geo::intersection get_state() const { return state; }

    private:
        const copyable_scene_data& scene;
        geo::intersection state;
    } callback(voxelised.get_scene_data());

    traverse(voxelised.get_voxels(), ray, callback);
    return callback.get_state();
}

bool inside(const voxelised_scene_data& voxelised, const glm::vec3& pt) {
    //  TODO test, might not modify state properly
    class callback final {
    public:
        callback(const copyable_scene_data& scene)
                : scene(scene) {}

        bool operator()(const geo::ray& ray,
                        const aligned::vector<size_t>& to_test,
                        float max_dist_inside_voxel) {
            for (const auto i : to_test) {
                const auto intersection = triangle_intersection(
                        scene.get_triangles()[i], scene.get_vertices(), ray);
                if (intersection && *intersection < max_dist_inside_voxel) {
                    if (proc::find_if(distances, [&intersection](auto i) {
                            return almost_equal(i, *intersection, size_t{10});
                        }) == distances.end()) {
                        distances.push_back(*intersection);
                    }
                }
            }
            return false;
        }

        bool get_state() const { return distances.size() % 2; }

    private:
        const copyable_scene_data& scene;
        aligned::vector<float> distances;
    } callback(voxelised.get_scene_data());

    const geo::ray ray{pt, glm::vec3{0, 0, 1}};
    traverse(voxelised.get_voxels(), ray, callback);
    return callback.get_state();
}
