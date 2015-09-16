#include "iterative_tetrahedral_mesh.h"

#include "logger.h"

#include <algorithm>

using namespace std;

IterativeTetrahedralMesh::Locator::Locator(const Vec3i & pos, int mod_ind)
        : pos(pos)
        , mod_ind(mod_ind) {
}

vector<Vec3f> IterativeTetrahedralMesh::get_scaled_cube() const {
    const vector<Vec3f> basic_cube{
        {0.00, 0.00, 0.00},
        {0.50, 0.00, 0.50},
        {0.25, 0.25, 0.25},
        {0.75, 0.25, 0.75},
        {0.00, 0.50, 0.50},
        {0.50, 0.50, 0.00},
        {0.25, 0.75, 0.75},
        {0.75, 0.75, 0.25},
    };
    vector<Vec3f> ret(basic_cube.size());
    transform(basic_cube.begin(),
              basic_cube.end(),
              ret.begin(),
              [this](auto i) { return i * spacing; });
    return ret;
}

Vec3i IterativeTetrahedralMesh::get_dim() const {
    auto dimensions = boundary.get_dimensions();
    return (dimensions / spacing).map([](auto i) { return ceil(i); }) + 1;
}

vector<TetrahedralNode> IterativeTetrahedralMesh::get_nodes(
    const Boundary & boundary) const {
    auto total_nodes = dim.product() * scaled_cube.size();
    vector<TetrahedralNode> ret(total_nodes);
    auto counter = 0u;
    transform(ret.begin(),
              ret.end(),
              ret.begin(),
              [this, &counter, &boundary](auto i) {
                  TetrahedralNode ret;
                  ret.inside =
                      boundary.inside(get_position(get_locator(counter++)));
                  return ret;
              });
    Logger::log(total_nodes, " nodes tested against boundary!");
    Logger::log(
        count_if(ret.begin(), ret.end(), [](auto i) { return i.inside; }),
        " nodes inside boundary");
    return ret;
}

IterativeTetrahedralMesh::IterativeTetrahedralMesh(const Boundary & boundary,
                                                   float spacing)
        : boundary(boundary.get_aabb())
        , spacing(spacing)
        , scaled_cube(get_scaled_cube())
        , dim(get_dim())
        , nodes(get_nodes(boundary)) {
}

IterativeTetrahedralMesh::size_type IterativeTetrahedralMesh::get_index(
    const Locator & loc) const {
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

Vec3f IterativeTetrahedralMesh::get_position(const Locator & locator) const {
    auto cube_pos = locator.pos * spacing;
    auto node_pos = scaled_cube[locator.mod_ind];
    return cube_pos + node_pos + boundary.c0;
}
