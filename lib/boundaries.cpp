#include "boundaries.h"

#include <cmath>
#include <algorithm>

using namespace std;

Vec3f convert(const cl_float3 & c) {
    return Vec3f(c.x, c.y, c.z);
}

CuboidBoundary::CuboidBoundary(const Vec3f & c0, const Vec3f & c1)
        : c0(c0)
        , c1(c1) {
}

bool CuboidBoundary::inside(const Vec3f & v) const {
    return (c0 < v).all() && (v < c1).all();
}

Vec3f CuboidBoundary::get_dimensions() const {
    return c1 - c0;
}

CuboidBoundary get_cuboid_boundary(const vector<cl_float3> & vertices) {
    Vec3f mini, maxi;
    mini = maxi = convert(vertices.front());
    for (auto i = vertices.begin() + 1; i != vertices.end(); ++i) {
        auto v = convert(*i);
        mini = v.binop(mini, [](auto a, auto b) {return min(a, b);});
        maxi = v.binop(maxi, [](auto a, auto b) {return max(a, b);});
    }
    return CuboidBoundary(mini, maxi);
}

SphereBoundary::SphereBoundary(const Vec3f & c, float radius)
        : c(c)
        , radius(radius) {
}

bool SphereBoundary::inside(const Vec3f & v) const {
    return (v - c).mag() < radius;
}

Vec3i MeshBoundary::hash_point(const Vec3f & v) const {
    return ((i + cuboid_boundary.c0) / cell_size).map([](auto j){
        return (int)floor(j);
    });
}

MeshBoundary::MeshBoundary(const vector<Triangle> & triangles,
                           const vector<cl_float3> & vertices)
        : triangles(triangles)
        , vertices(vertices)
        , triangle_references(DIVISIONS, vector<vector<uint32_t>>(DIVISIONS))
        , cuboid_boundary(get_cuboid_boundary(vertices))
        , cell_size(cuboid_boundary.get_dimensions() / DIVISIONS) {

    for (auto i = 0u; i != triangles.size(); ++i) {
        const auto & tri = triangles[i];
        auto bounding_box = get_cuboid_boundary({vertices[tri.v0],
                                                 vertices[tri.v1],
                                                 vertices[tri.v2]});
        auto get_indices = [&cuboid_boundary, &cell_size](auto i) {
        };

        Vec3i min_indices = get_indices(bounding_box.c0);
        Vec3i max_indices = get_indices(bounding_box.c1);

        for (auto j = min_indices.x; j != max_indices.x + 1; ++j) {
            for (auto k = min_indices.y; k != max_indices.y + 1; ++k) {
                triangle_references[j][j].push_back(i);
            }
        }
    }
}

bool MeshBoundary::inside(const Vec3f & v) const {
    //  hash v.xy to an index in triangle_references
    auto indices = hash_point(v);

    //  get triangle references
    auto references = triangle_references[indices.x][indices.y];

    //  TODO cast ray through point along Z axis and check for intersections
    //  with each of the referenced triangles

    //  TODO count number of intersections on one side of the point

    //  TODO if intersection number is even, point is outside, else it's inside
}
