#include "rectangular_mesh.h"

#include "conversions.h"
#include "boundary_adjust.h"

#include "logger.h"

#include <algorithm>
#include <numeric>

RectangularMesh::Collection RectangularMesh::compute_nodes(
    const Boundary& boundary,
    const RectangularProgram& program,
    cl::CommandQueue& queue) const {
    //  TODO this takes for ever, put it on GPU?
    auto total_nodes = get_dim().product();
    auto bytes = total_nodes * sizeof(RectangularMesh::CondensedNode);
    Logger::log_err(bytes >> 20, " MB required for node metadata storage!");

    auto ret = std::vector<Node>(total_nodes);

    auto counter = 0u;
    std::generate(
        ret.begin(),
        ret.end(),
        [this, &counter] {
            Node ret;
            auto p = this->compute_position(this->compute_locator(counter));
            this->compute_neighbors(counter, ret.ports);
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
                       i.boundary_type = RectangularProgram::id_none;
                       return i;
                   });

#if 0
    //------------------------------------------------------------------------//
    cl::Buffer node_buffer(
        program.getInfo<CL_PROGRAM_CONTEXT>(), ret.begin(), ret.end(), false);
    auto kernel = program.get_boundary_classify_kernel();
    for (auto set_bits = 0; set_bits != 3; ++set_bits) {
        kernel(cl::EnqueueArgs(queue, cl::NDRange(ret.size())),
               node_buffer,
               to_cl_int3(get_dim()),
               set_bits);
    }

    cl::copy(queue, node_buffer, ret.begin(), ret.end());
    //------------------------------------------------------------------------//
#else
    //------------------------------------------------------------------------//
    std::vector<cl_int> bt(ret.size());
    for (auto set_bits = 0; set_bits != 3; ++set_bits) {
        std::fill(bt.begin(), bt.end(), RectangularProgram::id_none);
        for (auto i = 0u; i != ret.size(); ++i) {
            auto& node = ret[i];
            if (!node.inside &&
                node.boundary_type == RectangularProgram::id_none) {
                for (auto j = 0; j != 6; ++j) {
                    auto port_ind = node.ports[j];
                    if (port_ind != Node::NO_NEIGHBOR) {
                        if ((!set_bits && ret[port_ind].inside) ||
                            (set_bits &&
                             ret[port_ind].boundary_type !=
                                 RectangularProgram::id_reentrant &&
                             popcount(ret[port_ind].boundary_type) ==
                                 set_bits)) {
                            bt[i] |= 1 << (j + 1);
                        }
                    }
                }
                auto final_bits = popcount(bt[i]);
                if (set_bits + 1 < final_bits) {
                    bt[i] = RectangularProgram::id_reentrant;
                } else if (final_bits <= set_bits) {
                    bt[i] = RectangularProgram::id_none;
                }
            } else {
                bt[i] = node.boundary_type;
            }
        }
        for (auto i = 0u; i != ret.size(); ++i) {
            ret[i].boundary_type = bt[i];
        }
    }
//------------------------------------------------------------------------//
#endif

    auto num_boundary_1 = 0;
    auto num_boundary_2 = 0;
    auto num_boundary_3 = 0;
    for (auto& i : ret) {
        switch (popcount(i.boundary_type)) {
            case 1:
                i.boundary_index = num_boundary_1++;
                break;
            case 2:
                i.boundary_index = num_boundary_2++;
                break;
            case 3:
                i.boundary_index = num_boundary_3++;
                break;
        }
    }

    return ret;
}

RectangularMesh::RectangularMesh(const RectangularProgram& program,
                                 cl::CommandQueue& queue,
                                 const Boundary& b,
                                 float spacing,
                                 const Vec3f& anchor)
        : BaseMesh(spacing,
                   compute_adjusted_boundary(b.get_aabb(), anchor, spacing))
        , dim(get_aabb().get_dimensions() / spacing)
        , nodes(compute_nodes(b, program, queue)) {
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
                       return inside
                                  ? compute_index(i)
                                  : RectangularProgram::NodeStruct::NO_NEIGHBOR;
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
