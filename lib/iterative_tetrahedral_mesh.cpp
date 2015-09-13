#include "iterative_tetrahedral_mesh.h"

#include <algorithm>

using namespace std;

IterativeTetrahedralMesh::Locator::Locator(const Vec3i & pos, int mod_ind)
        : pos(pos), mod_ind(mod_ind) {}

vector<Vec3f> get_scaled_cube(float scale) {
    const vector<Vec3f> basic_cube {
        {0.00,  0.00,   0.00},
        {0.50,  0.00,   0.50},
        {0.25,  0.25,   0.25},
        {0.75,  0.25,   0.75},
        {0.00,  0.50,   0.50},
        {0.50,  0.50,   0.00},
        {0.25,  0.75,   0.75},
        {0.75,  0.75,   0.25},
    };
    vector<Vec3f> ret(basic_cube.size());
    transform(basic_cube.begin(),
              basic_cube.end(),
              ret.begin(),
              [scale](auto i) { return i * scale; });
    return ret;
}

Vec3i get_dim(const Boundary & boundary, float cube_side) {
    auto dimensions = boundary.get_aabb().get_dimensions();
    return (dimensions / cube_side).map([](auto i){return ceil(i);}) + 1;
}

IterativeTetrahedralMesh::size_type get_index(
        const IterativeTetrahedralMesh::Locator & loc,
        const vector<Vec3f> & scaled_cube,
        const Vec3i & dim) {
    auto n = scaled_cube.size();
    return (loc.mod_ind) +
           (loc.pos.x * n) +
           (loc.pos.y * dim.x * n) +
           (loc.pos.z * dim.x * dim.y * n);
}

IterativeTetrahedralMesh::Locator get_locator(IterativeTetrahedralMesh::size_type index,
                                              const vector<Vec3f> & scaled_cube,
                                              const Vec3i & dim) {
    auto mod_ind = lldiv(index, scaled_cube.size());
    auto x       = lldiv(mod_ind.quot, dim.x);
    auto y       = lldiv(x.quot, dim.y);
    auto z       = lldiv(y.quot, dim.z);
    return IterativeTetrahedralMesh::Locator(Vec3i(x.rem, y.rem, z.rem), mod_ind.rem);
}

Vec3f get_position(IterativeTetrahedralMesh::Locator locator,
                   float spacing,
                   const vector<Vec3f> & scaled_cube) {
    auto cube_pos = locator.pos * spacing;
    auto node_pos = scaled_cube[locator.mod_ind];
    return cube_pos + node_pos;
}

vector<UnlinkedTetrahedralNode> get_nodes(const Boundary & boundary,
                                          float spacing,
                                          const vector<Vec3f> & scaled_cube,
                                          const Vec3i & dim) {
    auto total_nodes = dim.product() * scaled_cube.size();
    vector<UnlinkedTetrahedralNode> ret(total_nodes);
    auto counter = 0u;
    transform(ret.begin(),
              ret.end(),
              ret.begin(),
              [&counter, &boundary, &spacing, &scaled_cube, &dim](auto i) {
                  return UnlinkedTetrahedralNode{boundary.inside(get_position(get_locator(counter++, scaled_cube, dim), spacing, scaled_cube))};
              });
    return ret;
}

IterativeTetrahedralMesh::IterativeTetrahedralMesh(const Boundary & boundary,
                                                   float spacing)
        : spacing(spacing)
        , scaled_cube(get_scaled_cube(spacing))
        , dim(get_dim(boundary, spacing))
        , nodes(get_nodes(boundary, spacing, scaled_cube, dim)) {
}

IterativeTetrahedralMesh::size_type
IterativeTetrahedralMesh::get_index(const Locator & locator) const {
    return ::get_index(locator, scaled_cube, dim);
}

IterativeTetrahedralMesh::Locator
IterativeTetrahedralMesh::get_locator(size_type index) const {
    return ::get_locator(index, scaled_cube, dim);
}

Vec3f IterativeTetrahedralMesh::get_position(const Locator & locator) const {
    return ::get_position(locator, spacing, scaled_cube);
}

using Locator = IterativeTetrahedralMesh::Locator;

const vector<vector<Locator>> offset_table {
       {Locator(Vec3i(0, 0, 0), 2), Locator(Vec3i(-1, 0, -1), 3),
           Locator(Vec3i(-1, -1, 0), 6), Locator(Vec3i(0, -1, -1), 7)},
       {Locator(Vec3i(0, 0, 0), 2), Locator(Vec3i( 0, 0,  0), 3),
           Locator(Vec3i( 0, -1, 0), 6), Locator(Vec3i(0, -1,  0), 7)},
       {Locator(Vec3i(0, 0, 0), 0), Locator(Vec3i( 0, 0,  0), 1),
           Locator(Vec3i( 0,  0, 0), 4), Locator(Vec3i(0,  0,  0), 5)},
       {Locator(Vec3i(1, 0, 1), 0), Locator(Vec3i( 0, 0,  0), 1),
           Locator(Vec3i( 0,  0, 1), 4), Locator(Vec3i(1,  0,  0), 5)},
       {Locator(Vec3i(0, 0, 0), 2), Locator(Vec3i( 0, 0, -1), 3),
           Locator(Vec3i( 0,  0, 0), 6), Locator(Vec3i(0,  0, -1), 7)},
       {Locator(Vec3i(0, 0, 0), 2), Locator(Vec3i(-1, 0,  0), 3),
           Locator(Vec3i(-1,  0, 0), 6), Locator(Vec3i(0,  0,  0), 7)},
       {Locator(Vec3i(1, 1, 0), 0), Locator(Vec3i( 0, 1,  0), 1),
           Locator(Vec3i( 0,  0, 0), 4), Locator(Vec3i(1,  0,  0), 5)},
       {Locator(Vec3i(0, 1, 1), 0), Locator(Vec3i( 0, 1,  0), 1),
           Locator(Vec3i( 0,  0, 1), 4), Locator(Vec3i(0,  0,  0), 5)},
};

vector<int> IterativeTetrahedralMesh::get_neighbors(size_type index) const {
    auto locator = get_locator(index);
    auto relative = offset_table[locator.mod_ind];

    vector<int> ret;
    transform(relative.begin(),
              relative.end(),
              ret.begin(),
              [this, &locator](auto i) {
                  auto summed = locator.pos + i.pos;
                  return (Vec3i(0) <= summed && summed < dim).all() ? this->get_index(i) : -1;
              });

    return ret;
}
