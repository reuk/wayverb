#include "boundaries.h"
#include "conversions.h"
#include "geometric.h"
#include "scene_data.h"
#include "stl_wrappers.h"
#include "string_builder.h"
#include "test_flag.h"
#include "tri_cube_intersection.h"
#include "triangle.h"

//  serialization
#include "vec_serialize.h"

#include "glog/logging.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

CuboidBoundary::CuboidBoundary(const Box& b,
                               const std::vector<Surface>& surfaces)
        : Boundary(surfaces)
        , Box(b) {
}

CuboidBoundary::CuboidBoundary(const Vec3f& c0,
                               const Vec3f& c1,
                               const std::vector<Surface>& surfaces)
        : CuboidBoundary(Box(c0, c1), surfaces) {
}

bool CuboidBoundary::inside(const Vec3f& v) const {
    return (get_c0() < v).all() && (v < get_c1()).all();
}

bool CuboidBoundary::overlaps(const TriangleVec3f& t) const {
    auto coll = t;
    for (auto& i : coll) {
        i = (i - centre()) / dimensions();
    }
    return t_c_intersection(coll) == Rel::idInside;
}

CuboidBoundary CuboidBoundary::get_padded(float padding) const {
    return CuboidBoundary(
        get_c0() - padding, get_c1() + padding, get_surfaces());
}

CuboidBoundary CuboidBoundary::get_aabb() const {
    return *this;
}

SceneData CuboidBoundary::get_scene_data() const {
    std::vector<cl_float3> vertices{
        {{get_c0().x, get_c0().y, get_c0().z}},
        {{get_c1().x, get_c0().y, get_c0().z}},
        {{get_c0().x, get_c1().y, get_c0().z}},
        {{get_c1().x, get_c1().y, get_c0().z}},
        {{get_c0().x, get_c0().y, get_c1().z}},
        {{get_c1().x, get_c0().y, get_c1().z}},
        {{get_c0().x, get_c1().y, get_c1().z}},
        {{get_c1().x, get_c1().y, get_c1().z}},
    };
    std::vector<Triangle> triangles{
        {0, 0, 1, 5},
        {0, 0, 4, 5},
        {0, 0, 1, 3},
        {0, 0, 2, 3},
        {0, 0, 2, 6},
        {0, 0, 4, 6},
        {0, 1, 5, 7},
        {0, 1, 3, 7},
        {0, 2, 3, 7},
        {0, 2, 6, 7},
        {0, 4, 5, 7},
        {0, 4, 6, 7},
    };

    CHECK(!get_surfaces().empty()) << "boundary has no surfaces";

    return SceneData(triangles, vertices, get_surfaces());
}

bool CuboidBoundary::intersects(const geo::Ray& ray, float t0, float t1) {
    //  from http://people.csail.mit.edu/amy/papers/box-jgt.pdf
    auto inv = Vec3f(1) / ray.direction;
    std::array<bool, 3> sign{{inv.x < 0, inv.y < 0, inv.z < 0}};
    auto get_bounds = [this](auto i) { return i ? get_c0() : get_c1(); };

    auto tmin = (get_bounds(sign[0]).x - ray.position.x) * inv.x;
    auto tmax = (get_bounds(!sign[0]).x - ray.position.x) * inv.x;
    auto tymin = (get_bounds(sign[1]).y - ray.position.y) * inv.y;
    auto tymax = (get_bounds(!sign[1]).y - ray.position.y) * inv.y;
    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }
    if (tymin > tmin) {
        tmin = tymin;
    }
    if (tymax < tmax) {
        tmax = tymax;
    }
    auto tzmin = (get_bounds(sign[2]).z - ray.position.z) * inv.z;
    auto tzmax = (get_bounds(!sign[2]).z - ray.position.z) * inv.z;
    if ((tmin > tzmax) || (tzmin > tmax)) {
        return false;
    }
    if (tzmin > tmin) {
        tmin = tzmin;
    }
    if (tzmax < tmax) {
        tmax = tzmax;
    }
    return ((t0 < tmax) && (tmin < t1));
}

Box get_surrounding_box(const std::vector<Vec3f>& vertices) {
    Vec3f mini, maxi;
    mini = maxi = vertices.front();
    for (auto i = vertices.begin() + 1; i != vertices.end(); ++i) {
        mini = i->apply([](auto a, auto b) { return std::min(a, b); }, mini);
        maxi = i->apply([](auto a, auto b) { return std::max(a, b); }, maxi);
    }
    return Box(mini, maxi);
}

SphereBoundary::SphereBoundary(const Vec3f& c,
                               float radius,
                               const std::vector<Surface>& surfaces)
        : Boundary(surfaces)
        , c(c)
        , radius(radius)
        , boundary(Vec3f(-radius), Vec3f(radius)) {
}

bool SphereBoundary::inside(const Vec3f& v) const {
    return (v - c).mag() < radius;
}

CuboidBoundary SphereBoundary::get_aabb() const {
    return boundary;
}

Vec3i MeshBoundary::hash_point(const Vec3f& v) const {
    return ((v - boundary.get_c0()) / cell_size).map([](auto i) -> int {
        return floor(i);
    });
}

MeshBoundary::hash_table MeshBoundary::compute_triangle_references() const {
    hash_table ret(DIVISIONS, std::vector<reference_store>(DIVISIONS));
    for (auto& i : ret) {
        for (auto& j : i) {
            j.reserve(8);
        }
    }

    for (auto i = 0u; i != triangles.size(); ++i) {
        const auto& t = triangles[i];
        const auto bounding_box = get_surrounding_box(
            {vertices[t.v0], vertices[t.v1], vertices[t.v2]});
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

MeshBoundary::MeshBoundary(const std::vector<Triangle>& triangles,
                           const std::vector<Vec3f>& vertices,
                           const std::vector<Surface>& surfaces)
        : Boundary(surfaces)
        , triangles(triangles)
        , vertices(vertices)
        , boundary(get_surrounding_box(vertices))
        , cell_size(boundary.dimensions() / DIVISIONS)
        , triangle_references(compute_triangle_references()) {
#ifdef TESTING
    auto fname = build_string("./file-mesh.txt");
    ofstream file(fname);
    for (const auto& i : this->triangles) {
        auto v0 = this->vertices[i.v0];
        file << build_string(v0.x, " ", v0.y, " ", v0.z, " ");
        auto v1 = this->vertices[i.v1];
        file << build_string(v1.x, " ", v1.y, " ", v1.z, " ");
        auto v2 = this->vertices[i.v2];
        file << build_string(v2.x, " ", v2.y, " ", v2.z, " ");
        file << endl;
    }
#endif
}

std::tuple<std::vector<Triangle>, std::vector<Vec3f>> get_mesh_boundary_data(
    const SceneData& sd) {
    return std::make_tuple(sd.get_triangles(), sd.get_converted_vertices());
}

MeshBoundary::MeshBoundary(const SceneData& sd)
        : MeshBoundary(sd.get_triangles(),
                       sd.get_converted_vertices(),
                       sd.get_surfaces()) {
}

const MeshBoundary::reference_store& MeshBoundary::get_references(
    const Vec3i& i) const {
    return get_references(i.x, i.y);
}

const MeshBoundary::reference_store& MeshBoundary::get_references(int x,
                                                                  int y) const {
    if (0 <= x && x < DIVISIONS && 0 <= y && y < DIVISIONS) {
        return triangle_references[x][y];
    }
    return empty_reference_store;
}

bool MeshBoundary::inside(const Vec3f& v) const {
    //  cast ray through point along Z axis and check for intersections
    //  with each of the referenced triangles then count number of intersections
    //  on one side of the point
    //  if intersection number is even, point is outside, else it's inside
    const auto references = get_references(hash_point(v));
    geo::Ray ray(v, Vec3f(0, 0, 1));
    auto distances = std::vector<float>();
    return count_if(references.begin(),
                    references.end(),
                    [this, &ray, &distances](const auto& i) {
                        auto intersection =
                            triangle_intersection(triangles[i], vertices, ray);
                        if (intersection.intersects) {
                            auto already_in =
                                proc::find_if(
                                    distances, [&intersection](auto i) {
                                        return almost_equal(
                                            i, intersection.distance, 10);
                                    }) != distances.end();
                            distances.push_back(intersection.distance);
                            if (already_in) {
                                return false;
                            }
                        }
                        return intersection.intersects;
                    }) %
           2;
}

CuboidBoundary MeshBoundary::get_aabb() const {
    return boundary;
}

std::vector<int> MeshBoundary::get_triangle_indices() const {
    std::vector<int> ret(triangles.size());
    proc::iota(ret, 0);
    return ret;
}

const std::vector<Triangle>& MeshBoundary::get_triangles() const {
    return triangles;
}

const std::vector<Vec3f>& MeshBoundary::get_vertices() const {
    return vertices;
}

const CuboidBoundary& MeshBoundary::get_boundary() const {
    return boundary;
}

Vec3f MeshBoundary::get_cell_size() const {
    return cell_size;
}

constexpr int MeshBoundary::DIVISIONS;

const MeshBoundary::reference_store MeshBoundary::empty_reference_store{};
