#pragma once

#include "box.h"
#include "reduce.h"
#include "scene_data.h"
#include "triangle.h"
#include "triangle_vec.h"
#include "almost_equal.h"

#include "common/aligned/vector.h"

#include "glm/glm.hpp"

namespace geo {
class ray;
}  // namespace geo

class boundary {
public:
    boundary() = default;
    virtual ~boundary() noexcept = default;
    boundary(boundary&&) noexcept = default;
    boundary& operator=(boundary&&) noexcept = default;
    boundary(const boundary&) = default;
    boundary& operator=(const boundary&) = default;

    virtual bool inside(const glm::vec3& v) const = 0;
    virtual box get_aabb() const = 0;

    template <typename Archive>
    void serialize(Archive& archive);
};

class cuboid_boundary : public boundary {
public:
    cuboid_boundary() = default;
    cuboid_boundary(const glm::vec3& c0, const glm::vec3& c1);

    bool inside(const glm::vec3& v) const override;
    box get_aabb() const override;

    template <typename Archive>
    void serialize(Archive& archive);

private:
    box boundary;
};

class sphere_boundary : public boundary {
public:
    explicit sphere_boundary(const glm::vec3& c = glm::vec3(),
                             float radius = 0,
                             const aligned::vector<surface>& surfaces =
                                     aligned::vector<surface>{surface{}});
    bool inside(const glm::vec3& v) const override;
    box get_aabb() const override;

private:
    glm::vec3 c;
    float radius;
    box boundary;
};

class mesh_boundary : public boundary {
public:
    mesh_boundary(const aligned::vector<triangle>& triangles,
                  const aligned::vector<glm::vec3>& vertices,
                  const aligned::vector<surface>& surfaces);
    explicit mesh_boundary(const copyable_scene_data& sd);
    bool inside(const glm::vec3& v) const override;
    box get_aabb() const override;

    aligned::vector<size_t> get_triangle_indices() const;

    using reference_store = aligned::vector<uint32_t>;

    glm::ivec3 hash_point(const glm::vec3& v) const;
    const reference_store& get_references(int x, int y) const;
    const reference_store& get_references(const glm::ivec3& i) const;

    static constexpr int DIVISIONS = 1024;

    const aligned::vector<triangle>& get_triangles() const;
    const aligned::vector<glm::vec3>& get_vertices() const;
    const aligned::vector<surface>& get_surfaces() const;
    glm::vec3 get_cell_size() const;

private:
    using hash_table = aligned::vector<aligned::vector<reference_store>>;

    hash_table compute_triangle_references() const;

    aligned::vector<triangle> triangles;
    aligned::vector<glm::vec3> vertices;
    aligned::vector<surface> surfaces;
    box boundary;
    glm::vec3 cell_size;
    hash_table triangle_references;

    static const reference_store empty_reference_store;
};
