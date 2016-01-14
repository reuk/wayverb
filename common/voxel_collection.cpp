#include "voxel_collection.h"

#include <iostream>

VoxelCollection::Voxel::Voxel(const CuboidBoundary& aabb,
                              const std::vector<int>& triangles)
        : aabb(aabb)
        , triangles(triangles) {
}

VoxelCollection::Voxel::Voxel(const Octree& o)
        : Voxel(o.get_aabb(), o.get_triangles()) {
}

CuboidBoundary VoxelCollection::Voxel::get_aabb() const {
    return aabb;
}
const std::vector<int>& VoxelCollection::Voxel::Voxel::get_triangles() const {
    return triangles;
}

VoxelCollection::VoxelCollection(const Octree& o)
        : data(o.get_side(), YAxis(o.get_side(), ZAxis(o.get_side()))) {
    init(o);
}

VoxelCollection::VoxelCollection(const SceneData& scene_data,
                                 int depth,
                                 float padding)
        : VoxelCollection(Octree(scene_data, depth, padding)) {
}

void VoxelCollection::init(const Octree& o, const Vec3i& d) {
    if (o.get_nodes().empty()) {
        data[d.x][d.y][d.z] = Voxel(o);
    } else {
        auto count = 0u;
        for (const auto& i : o.get_nodes()) {
            auto x = (count & 1u) ? 1u : 0u;
            auto y = (count & 2u) ? 1u : 0u;
            auto z = (count & 4u) ? 1u : 0u;
            init(i, d + Vec3i(x, y, z) * o.get_side() / 2);
            count += 1;
        }
    }
}

const VoxelCollection::XAxis& VoxelCollection::get_data() const {
    return data;
}
