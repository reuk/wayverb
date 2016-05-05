#include "waveguide/rectangular_mesh.h"

#include "common/conversions.h"
#include "common/stl_wrappers.h"
#include "common/triangle.h"

#include "glog/logging.h"

#include <algorithm>
#include <numeric>

void RectangularMesh::set_node_positions(std::vector<Node>& ret) const {
    auto counter = 0u;
    proc::for_each(ret, [this, &counter](auto& node) {
        auto p = this->compute_position(this->compute_locator(counter));
        this->compute_neighbors(counter, node.ports);
        node.position = to_cl_float3(p);

        counter += 1;
    });
}

void RectangularMesh::set_node_inside(const Boundary& boundary,
                                      std::vector<Node>& ret) const {
    std::vector<bool> inside(ret.size());
    proc::transform(ret, inside.begin(), [&boundary](const auto& i) {
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

    proc::transform(ret,
                    inside.begin(),
                    inside.begin(),
                    [&neighbor_inside](auto node, auto in) {
                        return neighbor_inside(node) && in;
                    });

    proc::transform(ret, inside.begin(), ret.begin(), [](auto i, auto j) {
        i.inside = j;
        return i;
    });
}

constexpr RectangularMesh::Locator boundary_type_to_locator(
    RectangularProgram::BoundaryType b) {
    switch (b) {
        case RectangularProgram::id_nx:
            return RectangularMesh::Locator{-1, 0, 0};
        case RectangularProgram::id_px:
            return RectangularMesh::Locator{1, 0, 0};
        case RectangularProgram::id_ny:
            return RectangularMesh::Locator{0, -1, 0};
        case RectangularProgram::id_py:
            return RectangularMesh::Locator{0, 1, 0};
        case RectangularProgram::id_nz:
            return RectangularMesh::Locator{0, 0, -1};
        case RectangularProgram::id_pz:
            return RectangularMesh::Locator{0, 0, 1};
        default:
            return RectangularMesh::Locator{0, 0, 0};
    }
}

constexpr std::pair<RectangularMesh::Locator, cl_int> make_locator_pair() {
    return std::make_pair(RectangularMesh::Locator{0, 0, 0}, 0);
}

template <typename... Ts>
constexpr std::pair<RectangularMesh::Locator, cl_int> make_locator_pair(
    RectangularProgram::BoundaryType b, Ts... ts) {
    auto next = make_locator_pair(ts...);
    return std::make_pair(boundary_type_to_locator(b) + next.first,
                          b | next.second);
}

cl_int RectangularMesh::compute_boundary_type(
    const Locator& loc, const std::vector<Node>& ret) const {
    //  look at all nearby nodes

    using RectangularProgram::BoundaryType::id_nx;
    using RectangularProgram::BoundaryType::id_px;
    using RectangularProgram::BoundaryType::id_ny;
    using RectangularProgram::BoundaryType::id_py;
    using RectangularProgram::BoundaryType::id_nz;
    using RectangularProgram::BoundaryType::id_pz;
    using RectangularProgram::BoundaryType::id_none;
    using RectangularProgram::BoundaryType::id_reentrant;

    auto try_directions = [this, loc, &ret](
        const std::initializer_list<std::pair<Locator, cl_int>>& directions)
        -> cl_int {
            std::vector<std::pair<Locator, cl_int>> nearby;
            for (const auto& relative : directions) {
                auto adjacent = loc + relative.first;
                auto index = compute_index(adjacent);
                if (index < ret.size() && ret[index].inside)
                    nearby.push_back(relative);
            }
            if (nearby.size() == 1)
                return nearby.front().second;
            if (nearby.size() > 1)
                return id_reentrant;
            return id_none;
        };

    auto d1 = try_directions({
        make_locator_pair(id_nx),
        make_locator_pair(id_px),
        make_locator_pair(id_ny),
        make_locator_pair(id_py),
        make_locator_pair(id_nz),
        make_locator_pair(id_pz),
    });
    if (d1 != id_none)
        return d1;

    auto d2 = try_directions({
        make_locator_pair(id_nx, id_ny),
        make_locator_pair(id_px, id_ny),
        make_locator_pair(id_nx, id_py),
        make_locator_pair(id_px, id_py),
        make_locator_pair(id_nx, id_nz),
        make_locator_pair(id_px, id_nz),
        make_locator_pair(id_nx, id_pz),
        make_locator_pair(id_px, id_pz),
        make_locator_pair(id_ny, id_nz),
        make_locator_pair(id_py, id_nz),
        make_locator_pair(id_ny, id_pz),
        make_locator_pair(id_py, id_pz),
    });
    if (d2 != id_none)
        return d2;

    auto d3 = try_directions({
        make_locator_pair(id_nx, id_ny, id_nz),
        make_locator_pair(id_px, id_ny, id_nz),
        make_locator_pair(id_nx, id_py, id_nz),
        make_locator_pair(id_px, id_py, id_nz),
        make_locator_pair(id_nx, id_ny, id_pz),
        make_locator_pair(id_px, id_ny, id_pz),
        make_locator_pair(id_nx, id_py, id_pz),
        make_locator_pair(id_px, id_py, id_pz),
    });
    if (d3 != id_none)
        return d3;

    return id_none;
}

void RectangularMesh::set_node_boundary_type(std::vector<Node>& ret) const {
    for (auto i = 0u; i != ret.size(); ++i) {
        auto& node = ret[i];
        if (!node.inside) {
            node.boundary_type = compute_boundary_type(compute_locator(i), ret);
        }
    }
}

void RectangularMesh::set_node_boundary_index(std::vector<Node>& ret) const {
    set_node_boundary_index<1>(ret);
    set_node_boundary_index<2>(ret);
    set_node_boundary_index<3>(ret);
}

RectangularMesh::size_type RectangularMesh::compute_num_reentrant() const {
    return proc::count_if(get_nodes(), [](const auto& i) {
        return i.boundary_type == RectangularProgram::id_reentrant;
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
        (cube_pos -
         1).apply([](auto i, auto j) { return std::max(i, j); }, Vec3i(0));
    auto max =
        (cube_pos +
         2).apply([](auto i, auto j) { return std::min(i, j); }, get_dim());

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

    proc::transform(n_loc, output, [this](const auto& i) {
        auto inside = (Vec3i(0) <= i).all() && (i < dim).all();
        return inside ? compute_index(i) : RectangularProgram::NO_NEIGHBOR;
    });
}

std::vector<RectangularMesh::CondensedNode>
RectangularMesh::get_condensed_nodes() const {
    std::vector<RectangularMesh::CondensedNode> ret(get_nodes().size());
    proc::transform(get_nodes(), ret.begin(), [](const auto& i) {
        return RectangularProgram::condense(i);
    });
    proc::for_each(ret, [](const auto& i) {
        if ((i.boundary_type & RectangularProgram::id_inside) &&
            popcount(i.boundary_type) > 1) {
            LOG(INFO) << "too many bits set?";
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

cl_uint RectangularMesh::coefficient_index_for_node(
    const Boundary& b, const RectangularMesh::Node& node) {
    return 0;
}

cl_uint RectangularMesh::coefficient_index_for_node(
    const MeshBoundary& b, const RectangularMesh::Node& node) {
    const auto& triangles = b.get_triangles();
    const auto& vertices = b.get_vertices();
    auto min = proc::min_element(
        triangles, [&node, &vertices](const auto& i, const auto& j) {
            auto get_dist = [&node, &vertices](const auto& i) {
                return geo::point_triangle_distance_squared(
                    i, vertices, to_vec3f(node.position));
            };
            return get_dist(i) < get_dist(j);
        });
    //  set boundary data coefficient to triangle surface index
    return min->surface;
}
