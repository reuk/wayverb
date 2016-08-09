#include "common/spatial_division.h"
#include "common/scene_data.h"
#include "common/range.h"

octree octree_from_scene_data(const copyable_scene_data& scene_data,
                              size_t depth,
                              float padding) {
    return octree(
            depth,
            [&](auto item, const auto& aabb) {
                return geo::overlaps(
                        aabb,
                        geo::get_triangle_vec3(scene_data.get_triangles()[item],
                                               scene_data.get_vertices()));
            },
            scene_data.get_triangle_indices(),
            util::padded(scene_data.get_aabb(), glm::vec3(padding)));
}
