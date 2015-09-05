#include "bvh.h"

#include "vec.h"

#include <algorithm>
#include <cmath>

using namespace std;

const unsigned long T_SIDE = 128;
const unsigned long B_SIDE = 8;
const unsigned long T_SHIFT_FACTOR = 9;

auto convert(const cl_float3 & c) {
    return Vec3f(c.x, c.y, c.z);
}

Vec3f min(const Vec3f & a, const Vec3f & b) {
    return a.binop(b, [](auto i, auto j) {return min(i, j);});
}

Vec3f max(const Vec3f & a, const Vec3f & b) {
    return a.binop(b, [](auto i, auto j) {return max(i, j);});
}

auto get_centroid(const Triangle & t,
                  const vector<cl_float3> & vertices) {
    auto v0 = convert(vertices[t.v0]);
    auto v1 = convert(vertices[t.v1]);
    auto v2 = convert(vertices[t.v2]);
    return (v0 + v1 + v2) / 3;
}

uint32_t cell(const Vec3i & i, unsigned long div) {
    return i.x + i.y * div + i.z * div * div;
}

uint32_t t_cell(const Vec3i & i) {
    return cell(i, T_SIDE);
}

uint32_t b_cell(const Vec3i & i) {
    return cell(i, B_SIDE);
}

uint32_t get_cell(const Triangle & triangle,
                  const vector<cl_float3> & vertices,
                  const CuboidBoundary & boundary) {
    auto boundary_size = boundary.c1 - boundary.c0;

    auto centroid = get_centroid(triangle, vertices);

    auto top_grid_divisions = boundary_size / T_SIDE;
    auto bot_grid_divisions = boundary_size / B_SIDE;

    Vec3i top_grid_pos = (centroid / top_grid_divisions).map([](auto i) {return (int)floor(i);});
    Vec3i corrected_centroid = centroid - (top_grid_pos * top_grid_divisions);
    Vec3i bot_grid_pos = (corrected_centroid / bot_grid_divisions).map([](auto i) {return (int)floor(i);});

    return (t_cell(top_grid_pos) << T_SHIFT_FACTOR) | b_cell(bot_grid_pos);
}

struct TriReference {
    TriReference(uint32_t ref = 0, CuboidBoundary boundary = CuboidBoundary())
            : ref(ref), boundary(boundary) {}
    uint32_t ref;
    CuboidBoundary boundary;
};

struct CellID {
    CellID(uint32_t raw = 0): raw(raw) {}

    union {
        struct {
            uint32_t t_grid : 21;
            uint32_t b_grid : 9;
        };
        uint32_t raw;
    };
};

struct RefCellPair {
    RefCellPair(const TriReference & ref = 0, CellID cell = 0): ref(ref), cell(cell) {}
    TriReference ref;   //  offset into a const array/vector of Triangles
    CellID cell;        //  identifier for a bvh cell
};

CuboidBoundary get_cuboid_boundary(const vector<cl_float3> & vertices) {
    Vec3f mini, maxi;
    mini = maxi = convert(vertices.front());
    for (auto i = vertices.begin() + 1; i != vertices.end(); ++i) {
        auto v = convert(*i);
        mini = min(v, mini);
        maxi = max(v, maxi);
    }
    return CuboidBoundary(mini, maxi);
}

BVHMesh::BVHMesh(const vector<Triangle> & triangles,
                 const vector<cl_float3> & vertices)
        : triangles(triangles)
        , vertices(vertices)
        , boundary(get_cuboid_boundary(vertices)) {

    //  TODO
    //  Instead of just allocating a single TriReference for each triangle,
    //  we should divide each triangle into AABBs that are smaller than
    //  the minimum BVH node size, and then create a TriReference for each
    //  of these.
    vector<TriReference> tri_reference(triangles.size());
    auto counter = 0u;
    transform(triangles.begin(),
              triangles.end(),
              tri_reference.begin(),
              [&counter] (const auto & i) { return TriReference(counter++);});

    vector<RefCellPair> ref_cell_pairs(tri_reference.size());
    transform(tri_reference.begin(),
              tri_reference.end(),
              ref_cell_pairs.begin(),
              [this, &counter](const auto & i) {
                  return RefCellPair(i, get_cell(this->triangles[i.ref], this->vertices, boundary));
              });

    //  TODO this could be a GPU radix sort
    //  Sort the TriReferences by CellID
    sort(ref_cell_pairs.begin(), ref_cell_pairs.end(), [](auto a, auto b) {
        return a.cell.raw < b.cell.raw;
    });

    /* TODO
     * build top grid
     * build bottom grid
     * emit hierarchy
     * refit bounding boxes
     */
}
