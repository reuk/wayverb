#pragma once

#include "octree.h"

/// A box full of voxels, where each voxel keeps track of its own boundary
/// and indices of triangles that overlap that boundary.
/// Can be 'flattened' - converts the collection into a memory-efficient
/// array representation, which can be passed to the GPU.
class voxel_collection final {
public:
    class voxel final {
    public:
        explicit voxel(const box& aabb = box(),
                       const aligned::vector<size_t>& triangles =
                               aligned::vector<size_t>());
        voxel(const octree& o);
        box get_aabb() const;
        const aligned::vector<size_t>& get_triangles() const;

    private:
        box aabb;
        aligned::vector<size_t> triangles;
    };

    using z_axis = aligned::vector<voxel>;
    using y_axis = aligned::vector<z_axis>;
    using x_axis = aligned::vector<y_axis>;

    /// Construct from scene data. Uses an octree as an acceleration structure.
    voxel_collection(const copyable_scene_data& scene_data,
                     int depth,
                     float padding = 0.0f);
    /// Construct directly from an existing octree.
    voxel_collection(const octree& o);

    box get_aabb() const;
    box get_voxel_aabb() const;
    const x_axis& get_data() const;

    int get_side() const;

    const voxel& get_voxel(const glm::ivec3& i) const;

    glm::ivec3 get_starting_index(const glm::vec3& position) const;
    static glm::ivec3 get_step(const glm::vec3& direction);

    /// Returns a flat array-representation of the collection.
    /// TODO document the array format
    aligned::vector<cl_uint> get_flattened() const;

    class traversal_callback {
    public:
        traversal_callback() noexcept = default;
        virtual ~traversal_callback() noexcept = default;
        traversal_callback(traversal_callback&&) noexcept = default;
        traversal_callback& operator=(traversal_callback&&) noexcept = default;
        traversal_callback(const traversal_callback&) noexcept = default;
        traversal_callback& operator=(const traversal_callback&) noexcept =
                default;

        virtual geo::intersection operator()(
                const geo::ray& ray,
                const aligned::vector<size_t>& triangles) const = 0;
    };

    /// This callback is used to check for intersections with the contents of
    /// the collection.
    /// The colleciton doesn't store any information about the triangles it
    /// contains, so this callback is constructed with a reference to a
    /// SceneData object which contains the triangle information.
    class triangle_traversal_callback final : public traversal_callback {
    public:
        triangle_traversal_callback(const copyable_scene_data& scene_data);
        geo::intersection operator()(
                const geo::ray& ray,
                const aligned::vector<size_t>& triangles) const override;

    private:
        aligned::vector<triangle> tri;
        aligned::vector<glm::vec3> vertices;
    };

    /// Find the closest object along a ray.
    geo::intersection traverse(const geo::ray& ray,
                               const traversal_callback& fun) const;

private:
    void init(const octree& o, const glm::ivec3& offset = glm::ivec3(0));

    box aabb;
    x_axis data;
};
