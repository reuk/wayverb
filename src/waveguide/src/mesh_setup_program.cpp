#include "waveguide/mesh_setup_program.h"
#include "waveguide/cl/structs.h"
#include "waveguide/cl/utils.h"

#include "core/cl/geometry.h"
#include "core/cl/geometry_structs.h"
#include "core/cl/scene_structs.h"
#include "core/cl/voxel.h"

namespace waveguide {

constexpr auto source = R"(
int3 relative_locator_single(boundary_type a);
int3 relative_locator_single(boundary_type a) {
    switch (a) {
        case id_nx: return (int3)(-1, 0, 0);
        case id_px: return (int3)(1, 0, 0);
        case id_ny: return (int3)(0, -1, 0);
        case id_py: return (int3)(0, 1, 0);
        case id_nz: return (int3)(0, 0, -1);
        case id_pz: return (int3)(0, 0, 1);
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

constant int directions_2d[] = {id_nx | id_ny,
                                id_nx | id_py,
                                id_px | id_ny,
                                id_px | id_py,
                                id_nx | id_nz,
                                id_nx | id_pz,
                                id_px | id_nz,
                                id_px | id_pz,
                                id_ny | id_nz,
                                id_ny | id_pz,
                                id_py | id_nz,
                                id_py | id_pz};
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
    constant int* array;
    size_t size;
} direction_array_data;

int test_directions(int3 locator,
                    int3 dim,
                    direction_array_data data,
                    global condensed_node* nodes,
                    size_t num_nodes);
int test_directions(int3 locator,
                    int3 dim,
                    direction_array_data data,
                    global condensed_node* nodes,
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
        if (nodes[adjacent_index].boundary_type == id_inside) {
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

kernel void set_node_inside(global condensed_node* nodes,
                            const mesh_descriptor descriptor,

                            const global uint* voxel_index,  //  voxel
                            aabb global_aabb,
                            uint side,

                            const global triangle* triangles,  //  scene
                            const global float3* vertices) {
    //  find this thread index
    const size_t thread = get_global_id(0);

    //  zero out the return struct
    nodes[thread] = (condensed_node){};  //  zero it out to begin with

    //  find the 3d index of the node in the mesh
    const int3 locator = to_locator(thread, descriptor.dimensions);

    //  find its physical position
    const float3 position = compute_node_position(descriptor, locator);

    //  is the node's physical position inside the mesh?
    const bool inside = voxel_inside(
            position, voxel_index, global_aabb, side, triangles, vertices);

    //  if the node is inside
    if (inside) {
        //  signal that it inside
        nodes[thread].boundary_type = id_inside;
    }
}

kernel void set_node_boundary_type(global condensed_node* nodes,
                                   const mesh_descriptor descriptor) {
    const size_t thread = get_global_id(0);

    //  if the node is inside
    if (nodes[thread].boundary_type & id_inside) {
        return;
    }

    //  if we got here, the node is outside

    const size_t num_nodes = get_global_size(0);

    const direction_array_data data[] = {
            (direction_array_data){directions_1d, num_directions_1d},
            (direction_array_data){directions_2d, num_directions_2d},
            (direction_array_data){directions_3d, num_directions_3d}};
    const size_t num_data = sizeof(data) / sizeof(direction_array_data);

    const int3 locator = to_locator(thread, descriptor.dimensions);

    for (size_t i = 0; i != num_data; ++i) {
        const direction_array_data this_data = data[i];
        const int test = test_directions(
                locator, descriptor.dimensions, this_data, nodes, num_nodes);
        if (test != id_none) {
            atomic_xchg(&nodes[thread].boundary_type, test);
            return;
        }
    }
}

)";

setup_program::setup_program(const compute_context& cc)
        : program_wrapper(
                  cc,
                  std::vector<std::string>{
                          cl_representation_v<bands_type>,
                          cl_representation_v<surface<simulation_bands>>,
                          cl_representation_v<triangle>,
                          cl_representation_v<triangle_verts>,
                          cl_representation_v<boundary_type>,
                          cl_representation_v<condensed_node>,
                          cl_representation_v<mesh_descriptor>,
                          cl_representation_v<aabb>,
                          cl_representation_v<ray>,
                          cl_representation_v<triangle_inter>,
                          cl_representation_v<intersection>,
                          ::cl_sources::geometry,
                          ::cl_sources::voxel,
                          ::cl_sources::utils,
                          source}) {}

}  // namespace waveguide
