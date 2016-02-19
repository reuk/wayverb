#include "tetrahedral_mesh.h"

#include "conversions.h"
#include "boundary_adjust.h"

#include <algorithm>
#include <numeric>

TetrahedralLocator::TetrahedralLocator(const Vec3i& pos, int mod_ind)
        : pos(pos)
        , mod_ind(mod_ind) {
}

std::array<Vec3f, TetrahedralMesh::CUBE_NODES>
TetrahedralMesh::compute_scaled_cube(float scale) {
    const std::array<Vec3f, CUBE_NODES> basic_cube{{
        Vec3f(0.00, 0.00, 0.00),
        Vec3f(0.50, 0.00, 0.50),
        Vec3f(0.25, 0.25, 0.25),
        Vec3f(0.75, 0.25, 0.75),
        Vec3f(0.00, 0.50, 0.50),
        Vec3f(0.50, 0.50, 0.00),
        Vec3f(0.25, 0.75, 0.75),
        Vec3f(0.75, 0.75, 0.25),
    }};
    std::array<Vec3f, CUBE_NODES> ret;
    std::transform(basic_cube.begin(),
                   basic_cube.end(),
                   ret.begin(),
                   [scale](auto i) { return i * scale; });
    return ret;
}

const TetrahedralMesh::Collection& TetrahedralMesh::get_nodes() const {
    return nodes;
}

std::vector<TetrahedralMesh::Node> TetrahedralMesh::compute_nodes(
    const Boundary& boundary) const {
    auto total_nodes = get_dim().product() * scaled_cube.size();
    std::vector<Node> ret(total_nodes);
    auto counter = 0u;
    std::generate(
        ret.begin(),
        ret.end(),
        [this, &counter, &boundary] {
            Node ret;
            auto p = this->compute_position(this->compute_locator(counter));
            this->compute_neighbors(counter, ret.ports);
            ret.position = to_cl_float3(p);
            counter += 1;
            return ret;
        });

    std::vector<bool> inside(ret.size());
    std::transform(ret.begin(),
                   ret.end(),
                   inside.begin(),
                   [&boundary](const auto& i) {
                       return boundary.inside(to_vec3f(i.position));
                   });

    auto neighbor_inside = [&inside](const auto& i) {
        for (const auto& it : i.ports) {
            if (it != -1 && inside[it]) {
                return true;
            }
        }
        return false;
    };

    std::transform(ret.begin(),
                   ret.end(),
                   inside.begin(),
                   inside.begin(),
                   [&neighbor_inside](auto node, auto in) {
                       return neighbor_inside(node) && in;
                   });

    std::transform(ret.begin(),
                   ret.end(),
                   inside.begin(),
                   ret.begin(),
                   [](auto i, auto j) {
                       i.inside = j;
                       return i;
                   });

    return ret;
}

TetrahedralMesh::TetrahedralMesh(const Boundary& b,
                                 float spacing,
                                 const Vec3f& anchor)
        : TetrahedralMesh(
              b, spacing, anchor, cube_side_from_node_spacing(spacing)) {
}

TetrahedralMesh::TetrahedralMesh(const Boundary& b,
                                 float spacing,
                                 const Vec3f& anchor,
                                 float cube_side)
        : BaseMesh(spacing,
                   compute_adjusted_boundary(b.get_aabb(), anchor, cube_side))
        , scaled_cube(compute_scaled_cube(cube_side))
        , dim(get_aabb().get_dimensions() / cube_side)
        , nodes(compute_nodes(b)) {
}

TetrahedralMesh::size_type TetrahedralMesh::compute_index(
    const Locator& loc) const {
    auto n = scaled_cube.size();
    auto dim = get_dim();
    return (loc.mod_ind) + (loc.pos.x * n) + (loc.pos.y * dim.x * n) +
           (loc.pos.z * dim.x * dim.y * n);
}

TetrahedralMesh::Locator TetrahedralMesh::compute_locator(
    size_type index) const {
    auto mod_ind = div((int)index, (int)scaled_cube.size());
    auto dim = get_dim();
    auto x = div(mod_ind.quot, dim.x);
    auto y = div(x.quot, dim.y);
    auto z = div(y.quot, dim.z);
    return Locator(Vec3i(x.rem, y.rem, z.rem), mod_ind.rem);
}

TetrahedralMesh::Locator TetrahedralMesh::compute_locator(
    const Vec3f& v) const {
    auto transformed = v - get_aabb().get_c0();
    Vec3i cube_pos =
        (transformed / get_cube_side()).map([](auto i) -> int { return i; });

    auto min =
        (cube_pos - 1)
            .apply(Vec3i(0), [](auto i, auto j) { return std::max(i, j); });
    auto max =
        (cube_pos + 2)
            .apply(get_dim(), [](auto i, auto j) { return std::min(i, j); });

    std::vector<int> indices(scaled_cube.size());
    iota(indices.begin(), indices.end(), 0);

    auto get_dist = [this, v](auto loc) {
        return (v - compute_position(loc)).mag_squared();
    };

    Locator closest(min, 0);
    auto dist = get_dist(closest);
    for (auto x = min.x; x != max.x; ++x) {
        for (auto y = min.y; y != max.y; ++y) {
            for (auto z = min.z; z != max.z; ++z) {
                for (auto ind = 0u; ind != scaled_cube.size(); ++ind) {
                    Locator t(Vec3i(x, y, z), ind);
                    auto t_dist = get_dist(t);
                    if (t_dist < dist) {
                        closest = t;
                        dist = t_dist;
                    }
                }
            }
        }
    }

    return closest;
}

Vec3f TetrahedralMesh::compute_position(const Locator& locator) const {
    auto cube_pos = locator.pos * get_cube_side();
    auto node_pos = scaled_cube[locator.mod_ind];
    return cube_pos + node_pos + get_aabb().get_c0();
}

using Locator = TetrahedralMesh::Locator;

//  TODO any way to test this?
const std::array<std::array<Locator, TetrahedralMesh::PORTS>,
                 TetrahedralMesh::CUBE_NODES> TetrahedralMesh::offset_table{{
    {{Locator(Vec3i(0, 0, 0), 2),
      Locator(Vec3i(-1, 0, -1), 3),
      Locator(Vec3i(0, -1, -1), 6),
      Locator(Vec3i(-1, -1, 0), 7)}},
    {{Locator(Vec3i(0, 0, 0), 2),
      Locator(Vec3i(0, 0, 0), 3),
      Locator(Vec3i(0, -1, 0), 6),
      Locator(Vec3i(0, -1, 0), 7)}},
    {{Locator(Vec3i(0, 0, 0), 0),
      Locator(Vec3i(0, 0, 0), 1),
      Locator(Vec3i(0, 0, 0), 4),
      Locator(Vec3i(0, 0, 0), 5)}},
    {{Locator(Vec3i(1, 0, 1), 0),
      Locator(Vec3i(0, 0, 0), 1),
      Locator(Vec3i(1, 0, 0), 4),
      Locator(Vec3i(0, 0, 1), 5)}},
    {{Locator(Vec3i(0, 0, 0), 2),
      Locator(Vec3i(-1, 0, 0), 3),
      Locator(Vec3i(0, 0, 0), 6),
      Locator(Vec3i(-1, 0, 0), 7)}},
    {{Locator(Vec3i(0, 0, 0), 2),
      Locator(Vec3i(0, 0, -1), 3),
      Locator(Vec3i(0, 0, -1), 6),
      Locator(Vec3i(0, 0, 0), 7)}},
    {{Locator(Vec3i(0, 1, 1), 0),
      Locator(Vec3i(0, 1, 0), 1),
      Locator(Vec3i(0, 0, 0), 4),
      Locator(Vec3i(0, 0, 1), 5)}},
    {{Locator(Vec3i(1, 1, 0), 0),
      Locator(Vec3i(0, 1, 0), 1),
      Locator(Vec3i(1, 0, 0), 4),
      Locator(Vec3i(0, 0, 0), 5)}},
}};

void TetrahedralMesh::compute_neighbors(size_type index,
                                        cl_uint* output) const {
    auto locator = compute_locator(index);
    for (auto i = 0u; i != PORTS; ++i) {
        auto relative = offset_table[locator.mod_ind][i];
        auto summed = locator.pos + relative.pos;
        auto is_neighbor = (Vec3i(0) <= summed && summed < get_dim()).all();
        output[i] =
            is_neighbor ? compute_index(Locator(summed, relative.mod_ind)) : -1;
    }
}

float TetrahedralMesh::cube_side_from_node_spacing(float spacing) {
    return spacing / Vec3f(0.25, 0.25, 0.25).mag();
}

float TetrahedralMesh::get_cube_side() const {
    return cube_side_from_node_spacing(get_spacing());
}
const std::array<Vec3f, TetrahedralMesh::CUBE_NODES>&
TetrahedralMesh::get_scaled_cube() const {
    return scaled_cube;
}
Vec3i TetrahedralMesh::get_dim() const {
    return dim;
}
