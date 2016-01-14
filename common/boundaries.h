#pragma once

#include "scene_data.h"
#include "geometric.h"

class CuboidBoundary;

class Boundary {
public:
    Boundary() noexcept = default;
    virtual ~Boundary() noexcept = default;
    Boundary(Boundary&&) noexcept = default;
    Boundary& operator=(Boundary&&) noexcept = default;
    Boundary(const Boundary&) noexcept = default;
    Boundary& operator=(const Boundary&) noexcept = default;
    virtual bool inside(const Vec3f& v) const = 0;
    virtual CuboidBoundary get_aabb() const = 0;
};

template <typename F, typename T>
Vec3f sub_elementwise(const T& coll, const F& f = F()) {
    return std::accumulate(std::begin(coll) + 1,
                           std::end(coll),
                           *(std::begin(coll)),
                           [&f](auto a, auto b) { return a.apply(b, f); });
}

template <typename T>
Vec3f get_max(const T& coll) {
    return sub_elementwise(coll, [](auto i, auto j) { return std::max(i, j); });
}

template <typename T>
Vec3f get_min(const T& coll) {
    return sub_elementwise(coll, [](auto i, auto j) { return std::min(i, j); });
}

CuboidBoundary get_cuboid_boundary(const std::vector<Vec3f>& vertices);

class CuboidBoundary : public Boundary {
public:
    CuboidBoundary(const Vec3f& c0 = Vec3f(), const Vec3f& c1 = Vec3f());
    bool inside(const Vec3f& v) const override;
    bool overlaps(const TriangleVec3f& t) const;
    CuboidBoundary get_aabb() const override;
    CuboidBoundary get_padded(float padding) const;
    Vec3f get_centre() const;
    Vec3f get_dimensions() const;
    bool intersects(const geo::Ray& ray, float t0, float t1);
    Vec3f c0, c1;
};

class SphereBoundary : public Boundary {
public:
    SphereBoundary(const Vec3f& c = Vec3f(), float radius = 0);
    bool inside(const Vec3f& v) const override;
    CuboidBoundary get_aabb() const override;

    Vec3f c;
    float radius;
    CuboidBoundary boundary;
};

class MeshBoundary : public Boundary {
public:
    MeshBoundary(
        const std::vector<Triangle>& triangles = std::vector<Triangle>(),
        const std::vector<Vec3f>& vertices = std::vector<Vec3f>());
    MeshBoundary(const SceneData& sd);
    bool inside(const Vec3f& v) const override;
    CuboidBoundary get_aabb() const override;

    std::vector<int> get_triangle_indices() const;

    using reference_store = std::vector<uint32_t>;

    Vec3i hash_point(const Vec3f& v) const;
    reference_store get_references(int x, int y) const;
    reference_store get_references(const Vec3i& i) const;

    const int DIVISIONS{1024};

    std::vector<Triangle> triangles;
    std::vector<Vec3f> vertices;
    CuboidBoundary boundary;
    Vec3f cell_size;
    std::vector<std::vector<reference_store>> triangle_references;

private:
    MeshBoundary(
        const std::tuple<std::vector<Triangle>, std::vector<Vec3f>>& data);
    std::vector<std::vector<reference_store>> get_triangle_references() const;
};
