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

vectors::vectors(aligned::vector<condensed_node>&& nodes,
                 aligned::vector<coefficients_canonical>&& coefficients,
                 aligned::vector<boundary_index_array_1>&& boundary_indices_1,
                 aligned::vector<boundary_index_array_2>&& boundary_indices_2,
                 aligned::vector<boundary_index_array_3>&& boundary_indices_3)
        : condensed_nodes(std::move(nodes))
        , coefficients(std::move(coefficients))
        , boundary_indices_1(std::move(boundary_indices_1))
        , boundary_indices_2(std::move(boundary_indices_2))
        , boundary_indices_3(std::move(boundary_indices_3)) {}

const aligned::vector<condensed_node>& vectors::get_condensed_nodes() const {
    return condensed_nodes;
}

const aligned::vector<coefficients_canonical>& vectors::get_coefficients()
        const {
    return coefficients;
}

//----------------------------------------------------------------------------//

constexpr auto source{R"(
kernel void set_node_position_and_neighbors(global node* nodes,
                                            int3 dim,
                                            float3 min_corner,
                                            float spacing) {
    //  This should be two functions, but this way we only have to compute
    //  the locator once

    const size_t thread = get_global_id(0);
    nodes[thread] = (node){};   //  zero it out to begin with

    const int3 locator = to_locator(thread, dim);

    nodes[thread].position = min_corner + convert_float3(locator) * spacing;

    for (int i = 0; i != PORTS; ++i) {
        nodes[thread].ports[i] = neighbor_index(locator, dim, i);
    }
}

//----------------------------------------------------------------------------//

kernel void set_node_inside(global node* nodes,

                            const global uint * voxel_index,   //  voxel
                            aabb global_aabb,
                            uint side,

                            const global triangle * triangles, //  scene
                            const global float3 * vertices) {
    const size_t thread = get_global_id(0);
    const float3 position = nodes[thread].position;
	const bool inside = voxel_inside(position,
                                     voxel_index,
                                     global_aabb,
                                     side,
                                     triangles,
                                     vertices);
    nodes[thread].inside = inside;
}

//----------------------------------------------------------------------------//

int3 relative_locator_single(boundary_type a);
int3 relative_locator_single(boundary_type a) {
    switch (a) {
        case id_nx: return (int3)(-1,  0,  0);
        case id_px: return (int3)( 1,  0,  0);
        case id_ny: return (int3)( 0, -1,  0);
        case id_py: return (int3)( 0,  1,  0);
        case id_nz: return (int3)( 0,  0, -1);
        case id_pz: return (int3)( 0,  0,  1);
        default: return (int3)(0);
    }
}

int3 relative_locator(int a);
int3 relative_locator(int a) {
    return relative_locator_single(a & id_nx) +
           relative_locator_single(a & id_px) +
           relative_locator_single(a & id_ny) +
           relative_locator_single(a & id_py) +
           relative_locator_single(a & id_nz) +
           relative_locator_single(a & id_pz);
}

constant int directions_1d[] = {id_nx, id_px, id_ny, id_py, id_nz, id_pz};
constant size_t num_directions_1d = sizeof(directions_1d) / sizeof(int);

constant int directions_2d[] = {id_nx | id_ny, id_nx | id_py,
                                id_px | id_ny, id_px | id_py,
                                id_nx | id_nz, id_nx | id_pz,
                                id_px | id_nz, id_px | id_pz,
                                id_ny | id_nz, id_ny | id_pz,
                                id_py | id_nz, id_py | id_pz};
constant size_t num_directions_2d = sizeof(directions_2d) / sizeof(int);

constant int directions_3d[] = {id_nx | id_ny | id_nz,
                                id_nx | id_ny | id_pz,
                                id_nx | id_py | id_nz,
                                id_nx | id_py | id_pz,
                                id_px | id_ny | id_nz,
                                id_px | id_ny | id_pz,
                                id_px | id_py | id_nz,
                                id_px | id_py | id_pz};
constant size_t num_directions_3d = sizeof(directions_3d) / sizeof(int);

typedef struct {
    constant int * array;
    size_t size;
} direction_array_data;

int test_directions(int3 locator,
                    int3 dim,
                    direction_array_data data,
                    global node* nodes,
                    size_t num_nodes);
int test_directions(int3 locator,
                    int3 dim,
                    direction_array_data data,
                    global node* nodes,
                    size_t num_nodes) {
    int ret = id_none;

    //  for each direction
    for (size_t i = 0; i != data.size; ++i) {
        const int this_direction = data.array[i];
        const int3 relative = relative_locator(this_direction);
        const int3 adjacent_locator = locator + relative;

        if (locator_outside(adjacent_locator, dim)) {
            continue;
        }

        const size_t adjacent_index = to_index(adjacent_locator, dim);

        //  if the adjacent node in that direction is within the mesh
        //  and if the node in that direction is inside the model
        if (nodes[adjacent_index].inside) {
            //  if more than one adjacent node is inside
            if (ret != id_none) {
                //  the node is reentrant
                return id_reentrant;
            }

            //  otherwise, this is the only adjacent node inside
            ret = this_direction;
        }
    }

    return ret;
}

kernel void set_node_boundary_type(global node* nodes, int3 dim) {
    const size_t thread = get_global_id(0);
    const bool inside = nodes[thread].inside;
    if (inside) {
        return;
    }

    //  if we got here, the node is outside

    const int3 this_locator = to_locator(thread, dim);
    const size_t num_nodes = dim.x * dim.y * dim.z;

    const direction_array_data data[] = {
        (direction_array_data){directions_1d, num_directions_1d},
        (direction_array_data){directions_2d, num_directions_2d},
        (direction_array_data){directions_3d, num_directions_3d}};
    const size_t num_data = sizeof(data) / sizeof(direction_array_data);

    for (size_t i = 0; i != num_data; ++i) {
        const direction_array_data this_data = data[i];
        const int test = test_directions(this_locator, dim, this_data, nodes, num_nodes);
        if (test != id_none) {
            nodes[thread].boundary_type = test;
            return;
        }
    }
}

)"};

setup_program::setup_program(const compute_context& cc)
        : program_wrapper(
                  cc,
                  std::vector<std::string>{cl_representation_v<volume_type>,
                                           cl_representation_v<surface>,
                                           cl_representation_v<triangle>,
                                           cl_representation_v<triangle_verts>,
                                           cl_representation_v<boundary_type>,
                                           cl_representation_v<node>,
                                           cl_representation_v<aabb>,
                                           cl_representation_v<ray>,
                                           cl_representation_v<triangle_inter>,
                                           cl_representation_v<intersection>,
                                           ::cl_sources::geometry,
                                           ::cl_sources::voxel,
                                           ::cl_sources::utils,
                                           source}) {}

}  // namespace mesh
}  // namespace waveguide
