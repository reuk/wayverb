#include "boundaries.h"
#include "logger.h"

#include <cmath>
#include <algorithm>

using namespace std;

CuboidBoundary::CuboidBoundary(const Vec3f& c0, const Vec3f& c1)
        : c0(c0)
        , c1(c1) {
}

bool CuboidBoundary::inside(const Vec3f& v) const {
    return (c0 < v).all() && (v < c1).all();
}

CuboidBoundary CuboidBoundary::get_aabb() const {
    return *this;
}

Vec3f CuboidBoundary::get_dimensions() const {
    return c1 - c0;
}

CuboidBoundary get_cuboid_boundary(const vector<Vec3f>& vertices) {
    Vec3f mini, maxi;
    mini = maxi = vertices.front();
    for (auto i = vertices.begin() + 1; i != vertices.end(); ++i) {
        mini = i->apply(mini, [](auto a, auto b) { return min(a, b); });
        maxi = i->apply(maxi, [](auto a, auto b) { return max(a, b); });
    }
    return CuboidBoundary(mini, maxi);
}

SphereBoundary::SphereBoundary(const Vec3f& c, float radius)
        : c(c)
        , radius(radius)
        , boundary(-radius, radius) {
}

bool SphereBoundary::inside(const Vec3f& v) const {
    return (v - c).mag() < radius;
}

CuboidBoundary SphereBoundary::get_aabb() const {
    return boundary;
}

Vec3i MeshBoundary::hash_point(const Vec3f& v) const {
    return ((v - boundary.c0) / cell_size)
        .map([](auto j) { return (int)floor(j); });
}

vector<vector<MeshBoundary::reference_store>>
MeshBoundary::get_triangle_references() const {
    vector<vector<reference_store>> ret(DIVISIONS,
                                        vector<reference_store>(DIVISIONS));
    for (auto i = 0u; i != triangles.size(); ++i) {
        auto bounding_box = get_cuboid_boundary({vertices[triangles[i].s[0]],
                                                 vertices[triangles[i].s[1]],
                                                 vertices[triangles[i].s[2]]});
        auto min_indices = hash_point(bounding_box.c0);
        auto max_indices = hash_point(bounding_box.c1)/* + 1*/;

        for (auto j = min_indices.x; j != max_indices.x/* && j != DIVISIONS*/;
             ++j) {
            for (auto k = min_indices.y; k != max_indices.y/* && k != DIVISIONS*/;
                 ++k) {
                ret[j][k].push_back(i);
            }
        }
    }
    return ret;
}

MeshBoundary::MeshBoundary(const vector<Triangle>& triangles,
                           const vector<Vec3f>& vertices)
        : triangles(triangles)
        , vertices(vertices)
        , boundary(get_cuboid_boundary(vertices))
        , cell_size(boundary.get_dimensions() / DIVISIONS)
        , triangle_references(get_triangle_references()) {
}

class Ray {
public:
    Ray(const Vec3f& position = Vec3f(), const Vec3f& direction = Vec3f())
            : position(position)
            , direction(direction) {
    }
    Vec3f position;
    Vec3f direction;
};

class Intersects {
public:
    Intersects()
            : intersects(false) {
    }

    Intersects(float distance)
            : intersects(true)
            , distance(distance) {
    }

    bool intersects;
    float distance;
};

std::ostream& operator<<(std::ostream& strm, const Intersects& obj) {
    return strm << "Intersects {" << obj.intersects << ", " << obj.distance
                << "}";
}

Intersects triangle_intersection(const Triangle& tri,
                                 const std::vector<Vec3f>& vertices,
                                 const Ray& ray) {
    auto EPSILON = 0.0001;

    auto v0 = vertices[tri.s[0]];
    auto v1 = vertices[tri.s[1]];
    auto v2 = vertices[tri.s[2]];

    auto e0 = v1 - v0;
    auto e1 = v2 - v0;

    auto pvec = ray.direction.cross(e1);
    auto det = e0.dot(pvec);

    if (-EPSILON < det && det < EPSILON)
        return Intersects();

    auto invdet = 1 / det;
    auto tvec = ray.position - v0;
    auto ucomp = invdet * tvec.dot(pvec);

    if (ucomp < 0 || 1 < ucomp)
        return Intersects();

    auto qvec = tvec.cross(e0);
    auto vcomp = invdet * ray.direction.dot(qvec);

    if (vcomp < 0 || 1 < vcomp + ucomp)
        return Intersects();

    return Intersects(invdet * e1.dot(qvec));
}

MeshBoundary::reference_store MeshBoundary::get_references(uint32_t x,
                                                           uint32_t y) const {
    if (x < triangle_references.size() && y < triangle_references[x].size())
        return triangle_references[x][y];
    return reference_store();
}

bool MeshBoundary::inside(const Vec3f& v) const {
    //  hash v.xy to an index in triangle_references
    const auto indices = hash_point(v);

    //  cast ray through point along Z axis and check for intersections
    //  with each of the referenced triangles then count number of intersections
    //  on one side of the point
    auto references = get_references(indices.x, indices.y);
    const Ray ray(v, Vec3f(0, 0, 1));
    auto count = count_if(
        references.begin(),
        references.end(),
        [this, &ray](const auto& i) {
            auto intersection =
                triangle_intersection(triangles[i], vertices, ray);
            return intersection.intersects && intersection.distance > 0;
        });

    //  if intersection number is even, point is outside, else it's inside
    return count % 2;
}

CuboidBoundary MeshBoundary::get_aabb() const {
    return boundary;
}

const int MeshBoundary::DIVISIONS = 1024;
