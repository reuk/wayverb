#include "tetrahedral.h"

#include "logger.h"

#include <cmath>
#include <algorithm>

using namespace std;

struct WorkingNode {
    WorkingNode(Vec3f position = Vec3f())
            : position(position) {
    }
    Vec3f position;
    std::vector<int> ports;
};

CuboidBoundary::CuboidBoundary(Vec3f c0, Vec3f c1)
        : c0(c0)
        , c1(c1) {
}

bool CuboidBoundary::inside(const Vec3f & v) const {
    return (c0 < v).all() && (v < c1).all();
}

vector<Vec3f> get_node_positions(bool inverted) {
    vector<Vec3f> ret{
        Vec3f(0, 2 * sqrt(2) / 3.0, 1 / 3.0),
        Vec3f(sqrt(2 / 3.0), -sqrt(2) / 3.0, 1 / 3.0),
        Vec3f(0, 0, -1),
        Vec3f(-sqrt(2 / 3.0), -sqrt(2) / 3.0, 1 / 3.0),
    };

    if (inverted) {
        transform(ret.begin(),
                  ret.end(),
                  ret.begin(),
                  [](const auto & i) { return i * Vec3f(-1, -1, 0); });
    }

    return ret;
}

void build_mesh(vector<WorkingNode> & ret,
                const Boundary & boundary,
                int parent_index,
                float spacing,
                bool inverted = false) {
    const auto & parent = ret[parent_index];

    //  generate list of new node locations to try
    auto next_positions = get_node_positions(inverted);

    vector<WorkingNode> next_nodes(next_positions.size());

    transform(next_positions.begin(),
              next_positions.end(),
              next_nodes.begin(),
              [spacing, &parent](const auto & i) {
                  return WorkingNode((i * spacing) + parent.position);
              });

    //  filter list on boundary.inside()
    next_nodes.erase(remove_if(next_nodes.begin(),
                               next_nodes.end(),
                               [&boundary](const auto & i) {
                                   return !boundary.inside(i.position);
                               }),
                     next_nodes.end());

    //  filter list on colliding Nodes
    //  TODO do this by recursive search back through parent nodes instead
    auto epsilon = 0.00001;
    for (auto & i : ret) {
        for (auto j = next_nodes.begin(); j != next_nodes.end();) {
            if ((j->position - epsilon < i.position).all() &&
                (i.position < j->position + epsilon).all()) {
                i.ports.push_back(parent_index);
                j = next_nodes.erase(j);
            } else {
                ++j;
            }
        }
    }

    //  push nodes into ret
    auto begin_ind = ret.size();
    ret.insert(ret.end(), next_nodes.begin(), next_nodes.end());
    auto end_ind = ret.size();

    //  update parent with indices of new nodes in ret
    for (auto i = begin_ind; i != end_ind; ++i) {
        ret[parent_index].ports.push_back(i);
    }

    for (auto i = begin_ind; i != end_ind; ++i) {
        build_mesh(ret, boundary, i, spacing, !inverted);
    }
}

vector<Node> tetrahedral_mesh(const Boundary & boundary,
                              Vec3f start,
                              float spacing) {
    vector<WorkingNode> temp{WorkingNode(start)};
    build_mesh(temp, boundary, 0, spacing);

    vector<Node> ret(temp.size());
    transform(temp.begin(),
              temp.end(),
              ret.begin(),
              [](const auto & i) {
                  Node ret;
                  for (auto j = 0; j != sizeof(Node::ports) / sizeof(cl_int);
                       ++j) {
                      ret.ports[j] = -1;
                  }
                  for (auto j = 0; j != i.ports.size(); ++j) {
                      ret.ports[j] = i.ports[j];
                  }
                  ret.position = {{i.position.x, i.position.y, i.position.z}};
                  return ret;
              });

    return ret;
}
