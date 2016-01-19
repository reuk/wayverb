#include "iterative_tetrahedral_mesh.h"

#include "test_flag.h"
#include "conversions.h"
#include "logger.h"

#include <algorithm>
#include <numeric>

IterativeTetrahedralMesh::Locator::Locator(const Vec3i& pos, int mod_ind)
        : pos(pos)
        , mod_ind(mod_ind) {
}

std::vector<Vec3f> IterativeTetrahedralMesh::get_scaled_cube() const {
    const std::vector<Vec3f> basic_cube{
        {0.00, 0.00, 0.00},
        {0.50, 0.00, 0.50},
        {0.25, 0.25, 0.25},
        {0.75, 0.25, 0.75},
        {0.00, 0.50, 0.50},
        {0.50, 0.50, 0.00},
        {0.25, 0.75, 0.75},
        {0.75, 0.75, 0.25},
    };
    std::vector<Vec3f> ret(basic_cube.size());
    transform(basic_cube.begin(),
              basic_cube.end(),
              ret.begin(),
              [this](auto i) { return i * cube_side; });
    return ret;
}

Vec3i IterativeTetrahedralMesh::get_dim() const {
    return boundary.get_dimensions() / cube_side;
}

//  TODO
//  sometimes my bedroom model looks like nodes that are oustide the model think
//  they are actually inside, which is rubbish

//  it could just be the drawing routines, but I'm not sure how to find out
//  right now

CuboidBoundary IterativeTetrahedralMesh::get_adjusted_boundary(
    const CuboidBoundary& min_boundary, const Vec3f& anchor) const {
    auto dif = anchor - min_boundary.get_c0();
    Vec3i ceiled = (dif / cube_side).map([](auto i) { return ceil(i); });
    auto extra = 6;
    auto c0 = anchor - ((ceiled + 1 + extra) * cube_side);
    Vec3i dim = ((min_boundary.get_c1() - c0) / cube_side)
                    .map([](auto i) { return ceil(i); }) +
                extra;
    auto c1 = c0 + dim * cube_side;
    return CuboidBoundary(c0, c1);
}

std::vector<KNode> IterativeTetrahedralMesh::get_nodes(
    const Boundary& boundary) const {
    auto total_nodes = dim.product() * scaled_cube.size();
    std::vector<KNode> ret(total_nodes);
    auto counter = 0u;
    std::generate(ret.begin(),
                  ret.end(),
                  [this, &counter, &boundary]() {
                      KNode ret;
                      auto p = this->get_position(this->get_locator(counter));
                      auto neighbors = this->get_neighbors(counter);
                      std::copy(neighbors.begin(),
                                neighbors.end(),
                                std::begin(ret.ports));
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

    std::transform(ret.begin(),
                   ret.end(),
                   inside.begin(),
                   ret.begin(),
                   [&inside](auto i, const auto& j) {
                       i.inside = id_outside;
                       if (j) {
                           i.inside = id_inside;
                       } else {
                           for (const auto& it : i.ports) {
                               if (it != -1 && inside[it]) {
                                   i.inside = id_boundary;
                                   break;
                               }
                           }
                       }
                       return i;
                   });

    for (const auto& i : ret) {
        auto p = to_vec3f(i.position);
        if (i.inside == id_inside && !boundary.get_aabb().inside(p)) {
            std::cout << "what on earth" << std::endl;
            std::cout << p << std::endl;
        }
    }

    return ret;
}

IterativeTetrahedralMesh::IterativeTetrahedralMesh(const Boundary& boundary,
                                                   float spacing,
                                                   const Vec3f& anchor)
        : cube_side(cube_side_from_node_spacing(spacing))
        , scaled_cube(get_scaled_cube())
        , boundary(get_adjusted_boundary(boundary.get_aabb(), anchor))
        , dim(get_dim())
        , nodes(get_nodes(boundary))
        , spacing(spacing) {
}

IterativeTetrahedralMesh::size_type IterativeTetrahedralMesh::get_index(
    const Locator& loc) const {
    auto n = scaled_cube.size();
    return (loc.mod_ind) + (loc.pos.x * n) + (loc.pos.y * dim.x * n) +
           (loc.pos.z * dim.x * dim.y * n);
}

IterativeTetrahedralMesh::Locator IterativeTetrahedralMesh::get_locator(
    size_type index) const {
    auto mod_ind = div((int)index, (int)scaled_cube.size());
    auto x = div(mod_ind.quot, dim.x);
    auto y = div(x.quot, dim.y);
    auto z = div(y.quot, dim.z);
    return IterativeTetrahedralMesh::Locator(Vec3i(x.rem, y.rem, z.rem),
                                             mod_ind.rem);
}

IterativeTetrahedralMesh::Locator IterativeTetrahedralMesh::get_locator(
    const Vec3f& v) const {
    auto transformed = v - boundary.get_c0();
    Vec3i cube_pos =
        (transformed / cube_side).map([](auto i) -> int { return i; });
    Vec3f difference = (transformed - (cube_pos * cube_side));

    std::vector<int> indices(scaled_cube.size());
    iota(indices.begin(), indices.end(), 0);
    auto closest =
        std::accumulate(indices.begin() + 1,
                        indices.end(),
                        indices.front(),
                        [this, difference](auto i, auto j) {
                            return (scaled_cube[i] - difference).mag() <
                                           (scaled_cube[j] - difference).mag()
                                       ? i
                                       : j;
                        });

    return Locator(cube_pos, closest);
}

Vec3f IterativeTetrahedralMesh::get_position(const Locator& locator) const {
    auto cube_pos = locator.pos * cube_side;
    auto node_pos = scaled_cube[locator.mod_ind];
    return cube_pos + node_pos + boundary.get_c0();
}

using Locator = IterativeTetrahedralMesh::Locator;

//  TODO any way to test this?
const std::array<std::array<Locator, IterativeTetrahedralMesh::PORTS>,
                 IterativeTetrahedralMesh::CUBE_NODES>
    IterativeTetrahedralMesh::offset_table{{
        {{Locator(Vec3i(0, 0, 0), 2),
          Locator(Vec3i(-1, 0, -1), 3),
          Locator(Vec3i(-1, -1, 0), 6),
          Locator(Vec3i(0, -1, -1), 7)}},
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
          Locator(Vec3i(0, 0, 1), 4),
          Locator(Vec3i(1, 0, 0), 5)}},
        {{Locator(Vec3i(0, 0, 0), 2),
          Locator(Vec3i(0, 0, -1), 3),
          Locator(Vec3i(0, 0, 0), 6),
          Locator(Vec3i(0, 0, -1), 7)}},
        {{Locator(Vec3i(0, 0, 0), 2),
          Locator(Vec3i(-1, 0, 0), 3),
          Locator(Vec3i(-1, 0, 0), 6),
          Locator(Vec3i(0, 0, 0), 7)}},
        {{Locator(Vec3i(1, 1, 0), 0),
          Locator(Vec3i(0, 1, 0), 1),
          Locator(Vec3i(0, 0, 0), 4),
          Locator(Vec3i(1, 0, 0), 5)}},
        {{Locator(Vec3i(0, 1, 1), 0),
          Locator(Vec3i(0, 1, 0), 1),
          Locator(Vec3i(0, 0, 1), 4),
          Locator(Vec3i(0, 0, 0), 5)}},
    }};

std::array<int, IterativeTetrahedralMesh::PORTS>
IterativeTetrahedralMesh::get_neighbors(size_type index) const {
    auto locator = get_locator(index);
    std::array<int, PORTS> ret;
    for (auto i = 0u; i != PORTS; ++i) {
        auto relative = offset_table[locator.mod_ind][i];
        auto summed = locator.pos + relative.pos;
        auto is_neighbor = (Vec3i(0) <= summed && summed < dim).all();
        ret[i] =
            is_neighbor ? get_index(Locator(summed, relative.mod_ind)) : -1;
    }
    return ret;
}

float IterativeTetrahedralMesh::cube_side_from_node_spacing(float spacing) {
    return spacing / Vec3f(0.25, 0.25, 0.25).mag();
}

const std::vector<KNode>& IterativeTetrahedralMesh::get_nodes() const {
    return nodes;
}

float IterativeTetrahedralMesh::get_spacing() const {
    return spacing;
}
