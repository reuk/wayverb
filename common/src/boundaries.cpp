#include "common/boundaries.h"

#include "common/aligned/vector.h"
#include "common/conversions.h"
#include "common/geometric.h"
#include "common/scene_data.h"
#include "common/stl_wrappers.h"
#include "common/string_builder.h"
#include "common/tri_cube_intersection.h"
#include "common/triangle.h"

#include "glog/logging.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

cuboid_boundary::cuboid_boundary(const glm::vec3& c0, const glm::vec3& c1)
        : boundary(c0, c1) {}

bool cuboid_boundary::inside(const glm::vec3& v) const {
    return ::inside(boundary, v);
}

box cuboid_boundary::get_aabb() const { return boundary; }

sphere_boundary::sphere_boundary(const glm::vec3& c,
                                 float radius,
                                 const aligned::vector<surface>& surfaces)
        : c(c)
        , radius(radius)
        , boundary(glm::vec3(-radius), glm::vec3(radius)) {}

bool sphere_boundary::inside(const glm::vec3& v) const {
    return glm::distance(v, c) < radius;
}

box sphere_boundary::get_aabb() const { return boundary; }

glm::ivec3 mesh_boundary::hash_point(const glm::vec3& v) const {
    return (v - boundary.get_c0()) / cell_size;
}

mesh_boundary::hash_table mesh_boundary::compute_triangle_references() const {
    hash_table ret(DIVISIONS, aligned::vector<reference_store>(DIVISIONS));
    for (auto& i : ret) {
        for (auto& j : i) {
            j.reserve(8);
        }
    }

    for (auto i = 0u; i != triangles.size(); ++i) {
        const auto& t = triangles[i];
        const std::array<glm::vec3, 3> coll{
                {vertices[t.v0], vertices[t.v1], vertices[t.v2]}};

        const auto bounding_box = min_max(std::begin(coll), std::end(coll));
        const auto min_indices  = hash_point(bounding_box.get_c0());
        const auto max_indices  = hash_point(bounding_box.get_c1()) + 1;

        for (auto j = min_indices.x; j != max_indices.x && j != DIVISIONS;
             ++j) {
            for (auto k = min_indices.y; k != max_indices.y && k != DIVISIONS;
                 ++k) {
                ret[j][k].push_back(i);
            }
        }
    }
    return ret;
}

mesh_boundary::mesh_boundary(const aligned::vector<Triangle>& triangles,
                             const aligned::vector<glm::vec3>& vertices,
                             const aligned::vector<surface>& surfaces)
        : triangles(triangles)
        , vertices(vertices)
        , surfaces(surfaces)
        , boundary(min_max(std::begin(vertices), std::end(vertices)))
        , cell_size(dimensions(boundary) / static_cast<float>(DIVISIONS))
        , triangle_references(compute_triangle_references()) {}

mesh_boundary::mesh_boundary(const copyable_scene_data& sd)
        : mesh_boundary(sd.get_triangles(),
                        sd.get_converted_vertices(),
                        sd.get_surfaces()) {}

const mesh_boundary::reference_store& mesh_boundary::get_references(
        const glm::ivec3& i) const {
    return get_references(i.x, i.y);
}

const mesh_boundary::reference_store& mesh_boundary::get_references(
        int x, int y) const {
    if (0 <= x && x < DIVISIONS && 0 <= y && y < DIVISIONS) {
        return triangle_references[x][y];
    }
    return empty_reference_store;
}

bool mesh_boundary::inside(const glm::vec3& v) const {
    //  cast ray through point along Z axis and check for intersections
    //  with each of the referenced triangles then count number of intersections
    //  on one side of the point
    //  if intersection number is even, point is outside, else it's inside
    const auto references = get_references(hash_point(v));
    geo::Ray ray(v, glm::vec3(0, 0, 1));
    auto distances = aligned::vector<float>();
    return count_if(references.begin(),
                    references.end(),
                    [this, &ray, &distances](const auto& i) {
                        auto intersection = triangle_intersection(
                                triangles[i], vertices, ray);
                        if (intersection) {
                            auto already_in =
                                    proc::find_if(
                                            distances, [&intersection](auto i) {
                                                return almost_equal(
                                                        i, *intersection, 10);
                                            }) != distances.end();
                            distances.push_back(*intersection);
                            if (already_in) {
                                return false;
                            }
                        }
                        return static_cast<bool>(intersection);
                    }) %
           2;
}

box mesh_boundary::get_aabb() const { return boundary; }

aligned::vector<size_t> mesh_boundary::get_triangle_indices() const {
    aligned::vector<size_t> ret(triangles.size());
    proc::iota(ret, 0);
    return ret;
}

const aligned::vector<Triangle>& mesh_boundary::get_triangles() const {
    return triangles;
}

const aligned::vector<glm::vec3>& mesh_boundary::get_vertices() const {
    return vertices;
}

const aligned::vector<surface>& mesh_boundary::get_surfaces() const {
    return surfaces;
}

glm::vec3 mesh_boundary::get_cell_size() const { return cell_size; }

constexpr int mesh_boundary::DIVISIONS;

const mesh_boundary::reference_store mesh_boundary::empty_reference_store{};
