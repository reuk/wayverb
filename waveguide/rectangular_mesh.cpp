#include "rectangular_mesh.h"

#include "conversions.h"

#include <algorithm>
#include <numeric>

void RectangularMesh::set_node_positions(std::vector<Node>& ret) const {
    auto counter = 0u;
    std::for_each(
        ret.begin(),
        ret.end(),
        [this, &counter](auto& node) {
            auto p = this->compute_position(this->compute_locator(counter));
            this->compute_neighbors(counter, node.ports);
            node.position = to_cl_float3(p);

            counter += 1;
        });
}

void RectangularMesh::set_node_inside(const Boundary& boundary,
                                      std::vector<Node>& ret) const {
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
}

template <int SET_BITS>
bool do_bt_transform(const RectangularMesh::Node& adjacent) {
//    return adjacent.boundary_type != RectangularProgram::id_none && popcount(adjacent.boundary_type) < SET_BITS;
    return adjacent.boundary_type != RectangularProgram::id_none && popcount(adjacent.boundary_type) == SET_BITS - 1;
}

template <int SET_BITS>
cl_int compute_single_node_boundary_type(
    const RectangularMesh::Node& node,
    const std::vector<RectangularMesh::Node>& ret) {
    //  for each adjacent node
    cl_int bt = RectangularProgram::id_none;
    std::array<cl_int, 6> stored_types{{RectangularProgram::id_none}};
    for (auto j = 0; j != 6; ++j) {
        //  find the index of the adjacent node
        auto port_ind = node.ports[j];
        //  if the node is part of the mesh
        //  and the node is next to a reentrant node or a node of n-1 order
        if (port_ind != RectangularProgram::NO_NEIGHBOR &&
            do_bt_transform<SET_BITS>(ret[port_ind])) {
            auto adj_bt = ret[port_ind].boundary_type;
            if (adj_bt != RectangularProgram::id_reentrant) {
                for (auto i = 0; i != j; ++i) {
                    if (stored_types[i] == adj_bt) {
                        return RectangularProgram::id_none;
                    }
                }
            }

            stored_types[j] = adj_bt;
        }
    }

    for (auto j = 0; j != 6; ++j) {
        if (stored_types[j] != RectangularProgram::id_none)
            bt |= RectangularProgram::port_index_to_boundary_type(j);
    }

    auto final_bits = popcount(bt);
    if (SET_BITS < final_bits) {
        return RectangularProgram::id_reentrant;
    } else if (final_bits < SET_BITS) {
        return RectangularProgram::id_none;
    }
    return bt;
}

//  in the 1d case
//  if node is next to one node that is 'inside' then it is a 1d boundary node
//  if node is next to more than one node that is 'inside' then it is reentrant
template <>
cl_int compute_single_node_boundary_type<1>(
    const RectangularMesh::Node& node,
    const std::vector<RectangularMesh::Node>& ret) {
    //  for each adjacent node
    cl_int bt = RectangularProgram::id_none;
    for (auto j = 0; j != 6; ++j) {
        //  find the index of the adjacent node
        auto port_ind = node.ports[j];
        //  if the node is part of the mesh and the adjacent node is inside
        if (port_ind != RectangularProgram::NO_NEIGHBOR &&
            ret[port_ind].inside) {
            // log this node
            bt |= RectangularProgram::port_index_to_boundary_type(j);
        }
    }

    //  if next to more than one inside node
    if (1 < popcount(bt)) {
        //  node is reentrant
        return RectangularProgram::id_reentrant;
    }
    return bt;
}

template <int SET_BITS>
void set_node_boundary_type(std::vector<RectangularMesh::Node>& ret) {
    std::vector<cl_int> bt(ret.size(), RectangularProgram::id_none);
    //  for each node
    //  set populate intermediate array
    for (auto i = 0u; i != ret.size(); ++i) {
        const auto& node = ret[i];
        if (!node.inside && node.boundary_type == RectangularProgram::id_none) {
            bt[i] = compute_single_node_boundary_type<SET_BITS>(ret[i], ret);
        } else {
            bt[i] = node.boundary_type;
        }
    }

    //  copy array contents
    for (auto i = 0u; i != ret.size(); ++i) {
        ret[i].boundary_type = bt[i];
    }
}

std::vector<std::pair<cl_int, cl_uint>>
RectangularMesh::compute_coefficient_indices(const Node& node) const {
    switch (popcount(node.boundary_type)) {
        case 1:
            return compute_coefficient_indices<1>(node);
        case 2:
            return compute_coefficient_indices<2>(node);
        case 3:
            return compute_coefficient_indices<3>(node);
    }
    throw std::runtime_error("invalid number of adjacent boundary nodes");
}

void RectangularMesh::log_node_stats(const std::vector<Node>& ret) const {
    Logger::log_err("total nodes: ", ret.size());
    Logger::log_err("unclassified nodes: ", std::count_if(ret.begin(), ret.end(), [](const auto& i) {return i.boundary_type == RectangularProgram::id_none;}));
    Logger::log_err("inside nodes: ", std::count_if(ret.begin(), ret.end(), [] (const auto& i) {return i.inside;}));
    Logger::log_err("reentrant nodes: ", compute_num_reentrant());
    Logger::log_err("1d nodes: ", compute_num_boundary<1>());
    Logger::log_err("2d nodes: ", compute_num_boundary<2>());
    Logger::log_err("3d nodes: ", compute_num_boundary<3>());
}

void RectangularMesh::set_node_boundary_type(std::vector<Node>& ret) const {
    log_node_stats(ret);
    ::set_node_boundary_type<1>(ret);
    log_node_stats(ret);
    ::set_node_boundary_type<2>(ret);
    log_node_stats(ret);
    ::set_node_boundary_type<3>(ret);
    log_node_stats(ret);
}

void RectangularMesh::set_node_boundary_index(std::vector<Node>& ret) const {
    set_node_boundary_index<1>(ret);
    set_node_boundary_index<2>(ret);
    set_node_boundary_index<3>(ret);
}

RectangularMesh::size_type RectangularMesh::compute_num_reentrant() const {
    return std::count_if(get_nodes().begin(),
                         get_nodes().end(),
                         [](const auto& i) {
                             return i.boundary_type ==
                                    RectangularProgram::id_reentrant;
                         });
}

RectangularMesh::size_type RectangularMesh::compute_index(
    const Locator& pos) const {
    return pos.x + pos.y * get_dim().x + pos.z * get_dim().x * get_dim().y;
}
RectangularMesh::Locator RectangularMesh::compute_locator(
    const size_type index) const {
    auto x = div(index, dim.x);
    auto y = div(x.quot, dim.y);
    return Locator(x.rem, y.rem, y.quot % dim.z);
}
RectangularMesh::Locator RectangularMesh::compute_locator(
    const Vec3f& v) const {
    auto transformed = v - get_aabb().get_c0();
    Vec3i cube_pos =
        (transformed / get_spacing()).map([](auto i) -> int { return i; });

    auto min =
        (cube_pos - 1)
            .apply(Vec3i(0), [](auto i, auto j) { return std::max(i, j); });
    auto max =
        (cube_pos + 2)
            .apply(get_dim(), [](auto i, auto j) { return std::min(i, j); });

    auto get_dist = [this, v](auto loc) {
        return (v - compute_position(loc)).mag_squared();
    };

    Locator closest = min;
    auto dist = get_dist(closest);
    for (auto x = min.x; x != max.x; ++x) {
        for (auto y = min.y; y != max.y; ++y) {
            for (auto z = min.z; z != max.z; ++z) {
                Locator t(x, y, z);
                auto t_dist = get_dist(t);
                if (t_dist < dist) {
                    closest = t;
                    dist = t_dist;
                }
            }
        }
    }

    return closest;
}
Vec3f RectangularMesh::compute_position(const Locator& locator) const {
    return locator * get_spacing() + get_aabb().get_c0();
}

void RectangularMesh::compute_neighbors(size_type index,
                                        cl_uint* output) const {
    auto loc = compute_locator(index);
    const std::array<Locator, RectangularMesh::PORTS> n_loc{{
        Locator(loc.x - 1, loc.y, loc.z),
        Locator(loc.x + 1, loc.y, loc.z),
        Locator(loc.x, loc.y - 1, loc.z),
        Locator(loc.x, loc.y + 1, loc.z),
        Locator(loc.x, loc.y, loc.z - 1),
        Locator(loc.x, loc.y, loc.z + 1),
    }};

    std::transform(std::begin(n_loc),
                   std::end(n_loc),
                   output,
                   [this](const auto& i) {
                       auto inside = (Vec3i(0) <= i).all() && (i < dim).all();
                       return inside ? compute_index(i)
                                     : RectangularProgram::NO_NEIGHBOR;
                   });
}

std::vector<RectangularMesh::CondensedNode>
RectangularMesh::get_condensed_nodes() const {
    std::vector<RectangularMesh::CondensedNode> ret(get_nodes().size());
    std::transform(
        get_nodes().begin(),
        get_nodes().end(),
        ret.begin(),
        [](const auto& i) { return RectangularProgram::condense(i); });
    std::for_each(
        ret.begin(),
        ret.end(),
        [](const auto& i) {
            if (all_flags_set(i.boundary_type,
                              std::make_tuple(RectangularProgram::id_inside)) &&
                popcount(i.boundary_type) > 1) {
                Logger::log_err("too many bits set?");
            }
        });
    return ret;
}

const RectangularMesh::Collection& RectangularMesh::get_nodes() const {
    return nodes;
}

Vec3i RectangularMesh::get_dim() const {
    return dim;
}

cl_int RectangularMesh::coefficient_index_for_node(
    const Boundary& b, const RectangularMesh::Node& node) {
    return 0;
}

cl_int RectangularMesh::coefficient_index_for_node(
    const MeshBoundary& b, const RectangularMesh::Node& node) {
    const auto& triangles = b.get_triangles();
    const auto& vertices = b.get_vertices();
    auto min =
        std::min_element(triangles.begin(),
                         triangles.end(),
                         [&node, &vertices](const auto& i, const auto& j) {
                             auto get_dist = [&node, &vertices](const auto& i) {
                                 return geo::point_triangle_distance_squared(
                                     i, vertices, to_vec3f(node.position));
                             };
                             return get_dist(i) < get_dist(j);
                         });
    //  set boundary data coefficient to triangle surface index
    return min->surface;
}