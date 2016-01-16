#pragma once

#include "octree.h"

#include <iostream>

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

    CuboidBoundary get_aabb() const;
    CuboidBoundary get_voxel_aabb() const;
    const XAxis& get_data() const;

    int get_side() const;

    const Voxel& get_voxel(const Vec3i& i) const;

    Vec3i get_starting_index(const Vec3f& position) const;
    static Vec3i get_step(const Vec3f& direction);

    std::vector<cl_uint> get_flattened() const;

    class TraversalCallback {
    public:
        TraversalCallback() noexcept = default;
        virtual ~TraversalCallback() noexcept = default;
        TraversalCallback(TraversalCallback&&) noexcept = default;
        TraversalCallback& operator=(TraversalCallback&&) noexcept = default;
        TraversalCallback(const TraversalCallback&) noexcept = default;
        TraversalCallback& operator=(const TraversalCallback&) noexcept =
            default;

        virtual geo::Intersection operator()(
            const geo::Ray& ray, const std::vector<int>& triangles) = 0;
    };

    class TriangleTraversalCallback final : public TraversalCallback {
    public:
        TriangleTraversalCallback(const SceneData& scene_data);
        geo::Intersection operator()(
            const geo::Ray& ray, const std::vector<int>& triangles) override;

    private:
        const std::vector<Triangle>& tri;
        std::vector<Vec3f> vertices;
    };

    geo::Intersection traverse(const geo::Ray& ray, TraversalCallback& fun);

private:
    void init(const Octree& o, const Vec3i& offset = Vec3i(0));

    CuboidBoundary aabb;
    XAxis data;
};
