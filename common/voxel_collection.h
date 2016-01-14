#pragma once

#include "octree.h"

class VoxelCollection final {
public:
    class Voxel final {
    public:
        Voxel(const CuboidBoundary& aabb = CuboidBoundary(),
              const std::vector<int>& triangles = std::vector<int>());
        Voxel(const Octree& o);
        CuboidBoundary get_aabb() const;
        const std::vector<int>& get_triangles() const;

    private:
        CuboidBoundary aabb;
        std::vector<int> triangles;
    };

    using ZAxis = std::vector<Voxel>;
    using YAxis = std::vector<ZAxis>;
    using XAxis = std::vector<YAxis>;

    VoxelCollection(const SceneData& scene_data,
                    int depth,
                    float padding = 0.0f);
    VoxelCollection(const Octree& o);

    const XAxis& get_data() const;

private:
    void init(const Octree& o, const Vec3i& offset = Vec3i(0));

    XAxis data;
};
