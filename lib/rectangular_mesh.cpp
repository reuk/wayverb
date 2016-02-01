#include "rectangular_mesh.h"

#include "conversions.h"
#include "boundary_adjust.h"

#include <algorithm>
#include <numeric>

RectangularMesh::Collection RectangularMesh::compute_nodes(
    const Boundary& boundary) const {
    //  TODO this takes for ever, put it on GPU?
    auto total_nodes = get_dim().product();
    auto ret = std::vector<Node>(total_nodes);

    auto counter = 0u;
    std::generate(
        ret.begin(),
        ret.end(),
        [this, &counter, &boundary] {
            Node ret;
            auto p = this->compute_position(this->compute_locator(counter));
            auto neighbors = this->compute_neighbors(counter);
            std::copy(
                neighbors.begin(), neighbors.end(), std::begin(ret.ports));
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

    std::vector<cl_int> bt(ret.size());
    for (auto& i : ret) {
        i.bt = id_none;
    }
    for (auto set_bits = 0; set_bits != 3; ++set_bits) {
        std::fill(bt.begin(), bt.end(), 0);
        for (auto i = 0u; i != ret.size(); ++i) {
            auto& node = ret[i];
            if (!node.inside && node.bt == id_none) {
                for (auto j = 0; j != 6; ++j) {
                    auto port_ind = node.ports[j];
                    if (0 <= port_ind) {
                        if ((!set_bits && inside[port_ind]) ||
                            (set_bits && ret[port_ind].bt != id_reentrant &&
                             popcount(ret[port_ind].bt) == set_bits)) {
                            bt[i] |= 1 << (j + 1);
                        }
                    }
                }
                auto final_bits = popcount(bt[i]);
                if (set_bits + 1 < final_bits) {
                    bt[i] = id_reentrant;
                } else if (final_bits <= set_bits) {
                    bt[i] = id_none;
                }
            } else {
                bt[i] = node.bt;
            }
        }
        for (auto i = 0u; i != ret.size(); ++i) {
            ret[i].bt = bt[i];
        }
    }

    return ret;
}

RectangularMesh::RectangularMesh(const Boundary& b,
                                 float spacing,
                                 const Vec3f& anchor)
        : BaseMesh(spacing,
                   compute_adjusted_boundary(b.get_aabb(), anchor, spacing))
        , dim(get_aabb().get_dimensions() / spacing)
        , nodes(compute_nodes(b)) {
}

RectangularMesh::size_type RectangularMesh::compute_index(
    const Locator& pos) const {
    return pos.x + pos.y * get_dim().x + pos.z * get_dim().x * get_dim().y;
}
RectangularMesh::Locator RectangularMesh::compute_locator(
    const size_type index) const {
    auto x = div(index, get_dim().x);
    auto y = div(x.quot, get_dim().y);
    auto z = div(y.quot, get_dim().z);
    return Locator(x.rem, y.rem, z.rem);
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

std::array<int, RectangularMesh::PORTS> RectangularMesh::compute_neighbors(
    size_type index) const {
    auto loc = compute_locator(index);
    std::array<Locator, RectangularMesh::PORTS> n_loc{{
        loc + Locator(-1, 0, 0),
        loc + Locator(1, 0, 0),
        loc + Locator(0, -1, 0),
        loc + Locator(0, 1, 0),
        loc + Locator(0, 0, -1),
        loc + Locator(0, 0, 1),
    }};

    std::array<int, RectangularMesh::PORTS> ret;
    for (auto i = 0u; i != RectangularMesh::PORTS; ++i) {
        auto inside = (Vec3i(0) <= n_loc[i] && n_loc[i] < dim).all();
        ret[i] = inside ? compute_index(n_loc[i]) : -1;
    }
    return ret;
}

const RectangularMesh::Collection& RectangularMesh::get_nodes() const {
    return nodes;
}
Vec3i RectangularMesh::get_dim() const {
    return dim;
}
