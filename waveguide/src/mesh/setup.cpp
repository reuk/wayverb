#include "waveguide/mesh/setup.h"
#include "common/cl/geometry.h"
#include "common/cl/scene_structs.h"
#include "common/cl/voxel.h"
#include "common/map_to_vector.h"
#include "common/popcount.h"
#include "waveguide/cl/utils.h"

namespace waveguide {
namespace mesh {

condensed_node get_condensed(const node& n) {
    const auto ret =
            condensed_node{n.boundary_type | (n.inside ? id_inside : id_none),
                           n.boundary_index};
    if ((ret.boundary_type & id_inside) && popcount(ret.boundary_type) > 1) {
        throw std::runtime_error(
                "probably too many bits set in condensed node boundary "
                "type");
    }
    return ret;
}

aligned::vector<condensed_node> get_condensed(const aligned::vector<node>& n) {
    return map_to_vector(n, [](const auto& i) { return get_condensed(i); });
}

//----------------------------------------------------------------------------//

vectors::vectors(const aligned::vector<node>& nodes,
                 const aligned::vector<canonical_coefficients>& coefficients)
        : condensed_nodes(get_condensed(nodes))
        , coefficients(coefficients) {}

const aligned::vector<condensed_node>& vectors::get_condensed_nodes() const {
    return condensed_nodes;
}

const aligned::vector<canonical_coefficients>& vectors::get_coefficients()
        const {
    return coefficients;
}

//----------------------------------------------------------------------------//

setup_program::setup_program(const cl::Context& context,
                             const cl::Device& device)
        : program_wrapper(context,
                          device,
                          std::vector<std::string>{::cl_sources::scene_structs,
                                                   ::cl_sources::geometry,
                                                   ::cl_sources::voxel,
                                                   cl_sources::utils,
                                                   source}) {}

const std::string setup_program::source{R"(

kernel void set_node_position_and_neighbors(global Node* nodes,
                                            int3 dim,
                                            float3 min_corner,
                                            float spacing) {
    //  This should be two functions, but this way we only have to compute
    //  the locator once

    const size_t thread = get_global_id(0);
    nodes[thread] = (Node){};   //  zero it out to begin with

    const int3 locator = to_locator(thread, dim);

    nodes[thread].position = min_corner + convert_float3(locator) * spacing;

    for (int i = 0; i != PORTS; ++i) {
        nodes[thread].ports[i] = neighbor_index(locator, dim, i);
    }
}

kernel void set_node_inside(global Node* nodes,

                            const global uint * voxel_index,   //  voxel
                            AABB global_aabb,
                            ulong side,

                            const global Triangle * triangles, //  scene
                            const global float3 * vertices) {
    const size_t thread = get_global_id(0);
    const float3 position = nodes[thread].position;
    nodes[thread].inside = voxel_inside(position,
                                        voxel_index,
                                        global_aabb,
                                        side,
                                        triangles,
                                        vertices);
}

)"};

}  // namespace mesh
}  // namespace waveguide
