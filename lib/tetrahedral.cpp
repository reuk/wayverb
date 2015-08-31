#include "tetrahedral.h"

using namespace std;

CuboidBoundary::CuboidBoundary(Vec3f c0, Vec3f c1)
        : c0(c0)
        , c1(c1) {
}

bool CuboidBoundary::inside(const Vec3f & v) const {
    return (c0 < v).all() && (v < c1).all();
}

void build_mesh(vector<Node> & ret,
                const Boundary & boundary,
                int parent_index,
                float spacing) {
    auto & parent = ret[parent_index];
    //  generate list of new node locations to try
    //  filter list on boundary.inside()
    //  filter list on colliding Nodes
    //      either by linear search through ret (simple but doesn't scale)
    //      or by recursively checking parent nodes up to a certain depth
    //  create nodes at valid positions
    //  push nodes into ret
    //  update parent with indices of new nodes in ret
}

vector<Node> tetrahedral_mesh(const Boundary & boundary, Vec3f start, float spacing) {
    vector<Node> ret{Node(start)};
    build_mesh(ret, boundary, 0, spacing);
    return ret;
}
