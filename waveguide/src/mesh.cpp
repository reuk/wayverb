#include "waveguide/mesh.h"

#include "common/conversions.h"
#include "common/stl_wrappers.h"
#include "common/triangle.h"

#include "glog/logging.h"

#include <algorithm>
#include <numeric>

namespace waveguide {

void mesh::set_node_positions(aligned::vector<program::NodeStruct>& ret) const {
    auto counter = 0u;
    proc::for_each(ret, [this, &counter](auto& node) {
        auto p = this->compute_position(this->compute_locator(counter));
        this->compute_neighbors(counter, node.ports);
        node.position = to_cl_float3(p);

        counter += 1;
    });
}

void mesh::set_node_inside(const boundary& boundary,
                           aligned::vector<program::NodeStruct>& ret) const {
    std::vector<bool> inside(ret.size());
    proc::transform(ret, inside.begin(), [&boundary](const auto& i) {
        return boundary.inside(to_vec3(i.position));
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

constexpr mesh::locator boundary_type_to_locator(program::BoundaryType b) {
    switch (b) {
        case program::id_nx: return mesh::locator{-1, 0, 0};
        case program::id_px: return mesh::locator{1, 0, 0};
        case program::id_ny: return mesh::locator{0, -1, 0};
        case program::id_py: return mesh::locator{0, 1, 0};
        case program::id_nz: return mesh::locator{0, 0, -1};
        case program::id_pz: return mesh::locator{0, 0, 1};
        default: return mesh::locator{0, 0, 0};
    }
}

constexpr std::pair<mesh::locator, cl_int> make_locator_pair() {
    return std::make_pair(mesh::locator{0, 0, 0}, 0);
}

template <typename... Ts>
constexpr std::pair<mesh::locator, cl_int> make_locator_pair(
        program::BoundaryType b, Ts... ts) {
    auto next = make_locator_pair(ts...);
    return std::make_pair(boundary_type_to_locator(b) + next.first,
                          b | next.second);
}

cl_int mesh::compute_boundary_type(
        const locator& loc,
        const aligned::vector<program::NodeStruct>& ret) const {
    //  look at all nearby nodes

    using program::BoundaryType::id_nx;
    using program::BoundaryType::id_px;
    using program::BoundaryType::id_ny;
    using program::BoundaryType::id_py;
    using program::BoundaryType::id_nz;
    using program::BoundaryType::id_pz;
    using program::BoundaryType::id_none;
    using program::BoundaryType::id_reentrant;

    auto try_directions = [this, loc, &ret](
            const std::initializer_list<std::pair<locator, cl_int>>& directions)
            -> cl_int {
        aligned::vector<std::pair<locator, cl_int>> nearby;
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

void mesh::set_node_boundary_type(
        aligned::vector<program::NodeStruct>& ret) const {
    for (auto i = 0u; i != ret.size(); ++i) {
        auto& node = ret[i];
        if (!node.inside) {
            node.condensed.boundary_type =
                    compute_boundary_type(compute_locator(i), ret);
        }
    }
}

void mesh::set_node_boundary_index(
        aligned::vector<program::NodeStruct>& ret) const {
    set_node_boundary_index<1>(ret);
    set_node_boundary_index<2>(ret);
    set_node_boundary_index<3>(ret);
}

size_t mesh::compute_num_reentrant() const {
    return proc::count_if(get_nodes(), [](const auto& i) {
        return i.condensed.boundary_type == program::id_reentrant;
    });
}

size_t mesh::compute_index(const locator& pos) const {
    return pos.x + pos.y * get_dim().x + pos.z * get_dim().x * get_dim().y;
}
mesh::locator mesh::compute_locator(size_t index) const {
    auto x = div(index, dim.x);
    auto y = div(x.quot, dim.y);
    return locator(x.rem, y.rem, y.quot % dim.z);
}
mesh::locator mesh::compute_locator(const glm::vec3& v) const {
    auto transformed = v - get_aabb().get_min();
    glm::ivec3 cube_pos = transformed / get_spacing();

    auto min = glm::max(cube_pos - 1, glm::ivec3(0));
    auto max = glm::min(cube_pos + 2, get_dim());

    auto get_dist = [this, v](auto loc) {
        return std::pow(glm::length(v - compute_position(loc)), 2);
    };

    locator closest = min;
    auto dist = get_dist(closest);
    for (auto x = min.x; x != max.x; ++x) {
        for (auto y = min.y; y != max.y; ++y) {
            for (auto z = min.z; z != max.z; ++z) {
                locator t(x, y, z);
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
glm::vec3 mesh::compute_position(const locator& locator) const {
    return glm::vec3(locator) * get_spacing() + get_aabb().get_min();
}

void mesh::compute_neighbors(size_t index, cl_uint* output) const {
    auto loc = compute_locator(index);
    const std::array<locator, num_ports> n_loc{{
            locator(loc.x - 1, loc.y, loc.z),
            locator(loc.x + 1, loc.y, loc.z),
            locator(loc.x, loc.y - 1, loc.z),
            locator(loc.x, loc.y + 1, loc.z),
            locator(loc.x, loc.y, loc.z - 1),
            locator(loc.x, loc.y, loc.z + 1),
    }};

    proc::transform(n_loc, output, [this](const auto& i) {
        auto inside = glm::all(glm::lessThanEqual(glm::ivec3(0), i)) &&
                      glm::all(glm::lessThan(i, dim));
        return inside ? compute_index(i) : program::NO_NEIGHBOR;
    });
}

std::array<cl_uint, mesh::num_ports> mesh::compute_neighbors(
        size_t index) const {
    std::array<cl_uint, mesh::num_ports> ret;
    compute_neighbors(index, ret.data());
    return ret;
}

aligned::vector<program::CondensedNodeStruct> mesh::get_condensed_nodes()
        const {
    aligned::vector<program::CondensedNodeStruct> ret;
    ret.reserve(get_nodes().size());
    proc::transform(get_nodes(), std::back_inserter(ret), [](const auto& i) {
        return i.get_condensed();
    });
    proc::for_each(ret, [](const auto& i) {
        if ((i.boundary_type & program::id_inside) &&
            popcount(i.boundary_type) > 1) {
            LOG(INFO) << "too many bits set?";
        }
    });
    return ret;
}

const aligned::vector<program::NodeStruct>& mesh::get_nodes() const {
    return nodes;
}

glm::ivec3 mesh::get_dim() const { return dim; }

cl_uint mesh::coefficient_index_for_node(const boundary& b,
                                         const program::NodeStruct& node) {
    return 0;
}

cl_uint mesh::coefficient_index_for_node(const mesh_boundary& b,
                                         const program::NodeStruct& node) {
    const auto& triangles = b.get_scene_data().get_triangles();
    const auto& vertices = b.get_scene_data().get_vertices();
    const auto min =
            proc::min_element(triangles, [&](const auto& i, const auto& j) {
                const auto get_dist = [&](const auto& i) {
                    return geo::point_triangle_distance_squared(
                            i, vertices, to_vec3(node.position));
                };
                return get_dist(i) < get_dist(j);
            });
    //  set boundary data coefficient to triangle surface index
    return min->surface;
}

float mesh::get_spacing() const { return spacing; }

geo::box mesh::get_aabb() const { return aabb; }

bool operator==(const mesh& a, const mesh& b) {
    return std::tie(a.dim,
                    a.nodes,
                    a.boundary_coefficients_1,
                    a.boundary_coefficients_2,
                    a.boundary_coefficients_3) ==
           std::tie(b.dim,
                    b.nodes,
                    b.boundary_coefficients_1,
                    b.boundary_coefficients_2,
                    b.boundary_coefficients_3);
}

bool operator!=(const mesh& a, const mesh& b) { return !(a == b); }

}  // namespace waveguide
