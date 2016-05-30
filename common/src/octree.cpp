#include "common/octree.h"

#include "common/stl_wrappers.h"
#include "common/tri_cube_intersection.h"
#include "common/triangle.h"

#include <algorithm>
#include <iostream>

static std::array<CuboidBoundary, 8> next_boundaries(
    const CuboidBoundary& parent) {
    auto centre = parent.centre();

    auto x0 = parent.get_c0().x;
    auto y0 = parent.get_c0().y;
    auto z0 = parent.get_c0().z;
    auto xc = centre.x;
    auto yc = centre.y;
    auto zc = centre.z;
    auto x1 = parent.get_c1().x;
    auto y1 = parent.get_c1().y;
    auto z1 = parent.get_c1().z;

    return std::array<CuboidBoundary, 8>{{
        CuboidBoundary(glm::vec3(x0, y0, z0), glm::vec3(xc, yc, zc)),
        CuboidBoundary(glm::vec3(xc, y0, z0), glm::vec3(x1, yc, zc)),
        CuboidBoundary(glm::vec3(x0, yc, z0), glm::vec3(xc, y1, zc)),
        CuboidBoundary(glm::vec3(xc, yc, z0), glm::vec3(x1, y1, zc)),
        CuboidBoundary(glm::vec3(x0, y0, zc), glm::vec3(xc, yc, z1)),
        CuboidBoundary(glm::vec3(xc, y0, zc), glm::vec3(x1, yc, z1)),
        CuboidBoundary(glm::vec3(x0, yc, zc), glm::vec3(xc, y1, z1)),
        CuboidBoundary(glm::vec3(xc, yc, zc), glm::vec3(x1, y1, z1)),
    }};
}

static std::vector<int> get_triangles(const CopyableSceneData& sd,
                                      const std::vector<int>& to_test,
                                      const CuboidBoundary& aabb) {
    std::vector<int> ret(to_test.size());
    ret.resize(proc::copy_if(to_test,
                             ret.begin(),
                             [&sd, &aabb](auto i) {
                                 return aabb.overlaps(get_triangle_verts(
                                     sd.get_triangles()[i], sd.get_vertices()));
                             }) -
               ret.begin());
    return ret;
}

static std::unique_ptr<std::array<Octree, 8>> get_nodes(
    const CopyableSceneData& sd,
    int md,
    const std::vector<int>& to_test,
    const CuboidBoundary& ab) {
    if (md == 0) {
        return nullptr;
    }

    auto next = next_boundaries(ab);
    auto ret = std::make_unique<std::array<Octree, 8>>();
    proc::transform(next, ret->begin(), [&sd, md, &to_test](const auto& i) {
        return Octree(sd, md - 1, to_test, i);
    });
    return ret;
}

Octree::Octree(const CopyableSceneData& sd, int md, float padding)
        : Octree(sd,
                 md,
                 sd.get_triangle_indices(),
                 sd.get_aabb().get_padded(padding)) {
}

Octree::Octree(const CopyableSceneData& sd,
               int md,
               const std::vector<int>& to_test,
               const CuboidBoundary& ab)
        : aabb(ab)
        , triangles(::get_triangles(sd, to_test, ab))
        , nodes(::get_nodes(sd, md, triangles, ab)) {
}

CuboidBoundary Octree::get_aabb() const {
    return aabb;
}

bool Octree::has_nodes() const {
    return nodes != nullptr;
}

const std::array<Octree, 8>& Octree::get_nodes() const {
    return *nodes;
}

const std::vector<int>& Octree::get_triangles() const {
    return triangles;
}

void Octree::fill_flattened(std::vector<FloatUInt>& ret) const {
    //  float[6]    aabb
    //  int         n triangles
    //  int[]       triangle indices
    //  int         n nodes
    //  int[]       node offsets

    const auto& triangles = get_triangles();

    ret.push_back(to_fui(get_aabb().get_c0().x));
    ret.push_back(to_fui(get_aabb().get_c0().y));
    ret.push_back(to_fui(get_aabb().get_c0().z));
    ret.push_back(to_fui(get_aabb().get_c1().x));
    ret.push_back(to_fui(get_aabb().get_c1().y));
    ret.push_back(to_fui(get_aabb().get_c1().z));

    if (!has_nodes()) {
        std::vector<FloatUInt> t(triangles.size() + 1);
        t[0].i = triangles.size();
        for (auto i = 0u; i != triangles.size(); ++i) {
            t[i + 1].i = triangles[i];
        }
        ret.insert(ret.end(), t.begin(), t.end());
    } else {
        const auto& nodes = get_nodes();
        ret.push_back(to_fui(0u));  //  no triangles

        ret.push_back(
            to_fui(static_cast<cl_uint>(nodes.size())));  //  some nodes
        auto node_table_start = ret.size();
        proc::for_each(nodes,
                       [&ret](const auto&) { ret.push_back(to_fui(0u)); });

        auto counter = node_table_start;
        proc::for_each(nodes, [&ret, &counter](const auto& i) {
            ret[counter++].i = ret.size();
            i.fill_flattened(ret);
        });
    }
}

std::vector<FloatUInt> Octree::get_flattened() const {
    std::vector<FloatUInt> ret;
    fill_flattened(ret);
    return ret;
}

std::vector<const Octree*> Octree::intersect(const geo::Ray& ray) const {
    auto& starting_node = get_surrounding_leaf(ray.position);
    std::vector<const Octree*> ret = {&starting_node};
    return ret;
}

const Octree& Octree::get_surrounding_leaf(const glm::vec3& v) const {
    if (!has_nodes()) {
        return *this;
    }

    auto c = get_aabb().centre();
    auto x = v.x > c.x ? 1u : 0u;
    auto y = v.y > c.y ? 2u : 0u;
    auto z = v.z > c.z ? 4u : 0u;
    return get_nodes().at(x | y | z).get_surrounding_leaf(v);
}

int Octree::get_side() const {
    if (!has_nodes()) {
        return 1;
    }
    return 2 * get_nodes().front().get_side();
}
