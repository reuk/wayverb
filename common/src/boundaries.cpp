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

CuboidBoundary::CuboidBoundary(const glm::vec3& c0, const glm::vec3& c1)
        : boundary(c0, c1) {}

bool CuboidBoundary::inside(const glm::vec3& v) const {
    return ::inside(boundary, v);
}

box CuboidBoundary::get_aabb() const { return boundary; }

SphereBoundary::SphereBoundary(const glm::vec3& c,
                               float radius,
                               const aligned::vector<Surface>& surfaces)
        : c(c)
        , radius(radius)
        , boundary(glm::vec3(-radius), glm::vec3(radius)) {}

bool SphereBoundary::inside(const glm::vec3& v) const {
    return glm::distance(v, c) < radius;
}

box SphereBoundary::get_aabb() const { return boundary; }

glm::ivec3 MeshBoundary::hash_point(const glm::vec3& v) const {
    return (v - boundary.get_c0()) / cell_size;
}

MeshBoundary::hash_table MeshBoundary::compute_triangle_references() const {
    hash_table ret(DIVISIONS, aligned::vector<reference_store>(DIVISIONS));
    for (auto& i : ret) {
        for (auto& j : i) {
            j.reserve(8);
        }
    }

    for (auto i = 0u; i != triangles.size(); ++i) {
        const auto& t           = triangles[i];
        const std::array<glm::vec3, 3> coll{
                {vertices[t.v0], vertices[t.v1], vertices[t.v2]}};

        const auto bounding_box = min_max(std::begin(coll), std::end(coll));
        const auto min_indices = hash_point(bounding_box.get_c0());
        const auto max_indices = hash_point(bounding_box.get_c1()) + 1;

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

MeshBoundary::MeshBoundary(const aligned::vector<Triangle>& triangles,
                           const aligned::vector<glm::vec3>& vertices,
                           const aligned::vector<Surface>& surfaces)
        : triangles(triangles)
        , vertices(vertices)
        , surfaces(surfaces)
        , boundary(min_max(std::begin(vertices), std::end(vertices)))
        , cell_size(dimensions(boundary) / static_cast<float>(DIVISIONS))
        , triangle_references(compute_triangle_references()) {}

MeshBoundary::MeshBoundary(const CopyableSceneData& sd)
        : MeshBoundary(sd.get_triangles(),
                       sd.get_converted_vertices(),
                       sd.get_surfaces()) {}

const MeshBoundary::reference_store& MeshBoundary::get_references(
        const glm::ivec3& i) const {
    return get_references(i.x, i.y);
}

const MeshBoundary::reference_store& MeshBoundary::get_references(int x,
                                                                  int y) const {
    if (0 <= x && x < DIVISIONS && 0 <= y && y < DIVISIONS) {
        return triangle_references[x][y];
    }
    return empty_reference_store;
}

bool MeshBoundary::inside(const glm::vec3& v) const {
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

box MeshBoundary::get_aabb() const { return boundary; }

aligned::vector<size_t> MeshBoundary::get_triangle_indices() const {
    aligned::vector<size_t> ret(triangles.size());
    proc::iota(ret, 0);
    return ret;
}

const aligned::vector<Triangle>& MeshBoundary::get_triangles() const {
    return triangles;
}

const aligned::vector<glm::vec3>& MeshBoundary::get_vertices() const {
    return vertices;
}

const aligned::vector<Surface>& MeshBoundary::get_surfaces() const {
    return surfaces;
}

glm::vec3 MeshBoundary::get_cell_size() const { return cell_size; }

constexpr int MeshBoundary::DIVISIONS;

const MeshBoundary::reference_store MeshBoundary::empty_reference_store{};
