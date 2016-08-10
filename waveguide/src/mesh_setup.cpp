#include "waveguide/mesh_setup.h"
#include "common/conversions.h"
#include "common/scene_data.h"
#include "waveguide/boundary_adjust.h"
#include "waveguide/mesh_boundary.h"

#include "cl/utils.h"

namespace waveguide {
namespace mesh_setup {

//template <typename B>
aligned::vector<node> do_mesh_setup(const cl::Context& context,
                                    const cl::Device& device,
                                    const copyable_scene_data& scene_data,
                                    const voxel_collection<3>& octree,
                                    float spacing,
                                    const glm::vec3& anchor) {
    program program(context, device);
    cl::CommandQueue queue(context, device);

    const auto aabb =
            compute_adjusted_boundary(octree.get_aabb(), anchor, spacing);
    const glm::ivec3 dim = dimensions(aabb) / spacing;

    const auto num_nodes = dim.x * dim.y * dim.z;
    cl::Buffer node_buffer(
            context, CL_MEM_READ_WRITE, num_nodes * sizeof(node));

    {
        auto kernel = program.get_node_position_and_neighbors_kernel();
        kernel(cl::EnqueueArgs(queue, cl::NDRange(num_nodes)),
               node_buffer,
               to_cl_int3(dim),
               to_cl_float3(aabb.get_min()),
               spacing);
    }

    aligned::vector<node> ret(num_nodes);
    cl::copy(queue, node_buffer, ret.begin(), ret.end());
    return ret;
}

program::program(const cl::Context& context, const cl::Device& device)
        : program_wrapper(context,
                          device,
                          std::vector<std::string>{cl_sources::utils, source}) {
}

const std::string program::source{R"(

kernel void set_node_position_and_neighbors(global Node* nodes,
                                            int3 dim,
                                            float3 min_corner,
                                            float spacing) {
    //  This should be two functions, but this way we only have to compute
    //  the locator once

    const size_t thread = get_global_id(0);
    const int3 locator = to_locator(thread, dim);
    nodes[thread].position = min_corner + (locator * (float3)(spacing));
    for (int i = 0; i != PORTS; ++i) {
        nodes[thread].ports[i] = neighbor_index(locator, dim, i);
    }
}

)"};

}  // namespace mesh_setup
}  // namespace waveguide
