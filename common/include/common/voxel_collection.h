#pragma once

#include "octree.h"

/// A box full of voxels, where each voxel keeps track of its own boundary
/// and indices of triangles that overlap that boundary.
/// Can be 'flattened' - converts the collection into a memory-efficient
/// array representation, which can be passed to the GPU.
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

    /// Construct from scene data. Uses an octree as an acceleration structure.
    VoxelCollection(const CopyableSceneData& scene_data,
                    int depth,
                    float padding = 0.0f);
    /// Construct directly from an existing octree.
    VoxelCollection(const Octree& o);

    CuboidBoundary get_aabb() const;
    CuboidBoundary get_voxel_aabb() const;
    const XAxis& get_data() const;

    int get_side() const;

    const Voxel& get_voxel(const glm::ivec3& i) const;

    glm::ivec3 get_starting_index(const glm::vec3& position) const;
    static glm::ivec3 get_step(const glm::vec3& direction);

    /// Returns a flat array-representation of the collection.
    /// TODO document the array format
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

    /// This callback is used to check for intersections with the contents of
    /// the collection.
    /// The colleciton doesn't store any information about the triangles it
    /// contains, so this callback is constructed with a reference to a
    /// SceneData object which contains the triangle information.
    class TriangleTraversalCallback final : public TraversalCallback {
    public:
        TriangleTraversalCallback(const CopyableSceneData& scene_data);
        geo::Intersection operator()(
            const geo::Ray& ray, const std::vector<int>& triangles) override;

    private:
        std::vector<Triangle> tri;
        std::vector<glm::vec3> vertices;
    };

    /// Find the closest object along a ray.
    geo::Intersection traverse(const geo::Ray& ray, TraversalCallback& fun);

private:
    void init(const Octree& o, const glm::ivec3& offset = glm::ivec3(0));

    CuboidBoundary aabb;
    XAxis data;
};
