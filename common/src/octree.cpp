#include "common/octree.h"

#include "common/stl_wrappers.h"
#include "common/tri_cube_intersection.h"
#include "common/triangle.h"

#include <algorithm>
#include <iostream>

namespace {
std::array<box, 8> next_boundaries(const box& parent) {
    const auto centre = ::centre(parent);

    const auto x0 = parent.get_c0().x;
    const auto y0 = parent.get_c0().y;
    const auto z0 = parent.get_c0().z;
    const auto xc = centre.x;
    const auto yc = centre.y;
    const auto zc = centre.z;
    const auto x1 = parent.get_c1().x;
    const auto y1 = parent.get_c1().y;
    const auto z1 = parent.get_c1().z;

    return std::array<box, 8>{{
            box(glm::vec3(x0, y0, z0), glm::vec3(xc, yc, zc)),
            box(glm::vec3(xc, y0, z0), glm::vec3(x1, yc, zc)),
            box(glm::vec3(x0, yc, z0), glm::vec3(xc, y1, zc)),
            box(glm::vec3(xc, yc, z0), glm::vec3(x1, y1, zc)),
            box(glm::vec3(x0, y0, zc), glm::vec3(xc, yc, z1)),
            box(glm::vec3(xc, y0, zc), glm::vec3(x1, yc, z1)),
            box(glm::vec3(x0, yc, zc), glm::vec3(xc, y1, z1)),
            box(glm::vec3(xc, yc, zc), glm::vec3(x1, y1, z1)),
    }};
}

aligned::vector<size_t> get_triangles(const CopyableSceneData& sd,
                                      const aligned::vector<size_t>& to_test,
                                      const box& aabb) {
    aligned::vector<size_t> ret(to_test.size());
    ret.resize(proc::copy_if(to_test,
                             ret.begin(),
                             [&sd, &aabb](auto i) {
                                 return overlaps(aabb,
                                                 get_triangle_verts(
                                                         sd.get_triangles()[i],
                                                         sd.get_vertices()));
                             }) -
               ret.begin());
    return ret;
}

std::unique_ptr<std::array<Octree, 8>> get_nodes(
        const CopyableSceneData& sd,
        int md,
        const aligned::vector<size_t>& to_test,
        const box& ab) {
    if (md == 0) {
        return nullptr;
    }

    const auto next = next_boundaries(ab);
    auto ret        = std::make_unique<std::array<Octree, 8>>();
    proc::transform(next, ret->begin(), [&sd, md, &to_test](const auto& i) {
        return Octree(sd, md - 1, to_test, i);
    });
    return ret;
}
}  // namespace

Octree::Octree(const CopyableSceneData& sd, size_t md, float padding)
        : Octree(sd,
                 md,
                 sd.get_triangle_indices(),
                 padded(sd.get_aabb(), padding)) {}

Octree::Octree(const CopyableSceneData& sd,
               size_t md,
               const aligned::vector<size_t>& to_test,
               const box& ab)
        : aabb(ab)
        , triangles(::get_triangles(sd, to_test, ab))
        , nodes(::get_nodes(sd, md, triangles, ab)) {}

box Octree::get_aabb() const { return aabb; }

bool Octree::has_nodes() const { return nodes != nullptr; }

const std::array<Octree, 8>& Octree::get_nodes() const { return *nodes; }

const aligned::vector<size_t>& Octree::get_triangles() const {
    return triangles;
}

aligned::vector<const Octree*> Octree::intersect(const geo::Ray& ray) const {
    const auto& starting_node = get_surrounding_leaf(ray.get_position());
    aligned::vector<const Octree*> ret = {&starting_node};
    return ret;
}

const Octree& Octree::get_surrounding_leaf(const glm::vec3& v) const {
    if (!has_nodes()) {
        return *this;
    }

    const auto c = centre(get_aabb());
    const auto x = v.x > c.x ? 1u : 0u;
    const auto y = v.y > c.y ? 2u : 0u;
    const auto z = v.z > c.z ? 4u : 0u;
    return get_nodes().at(x | y | z).get_surrounding_leaf(v);
}

size_t Octree::get_side() const {
    if (!has_nodes()) {
        return 1;
    }
    return 2 * get_nodes().front().get_side();
}
