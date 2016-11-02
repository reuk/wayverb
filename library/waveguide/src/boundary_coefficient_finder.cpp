#include "waveguide/boundary_coefficient_finder.h"
#include "waveguide/boundary_coefficient_program.h"
#include "waveguide/mesh.h"
#include "waveguide/setup.h"

namespace waveguide {

namespace {

template <typename it, typename func>
void set_boundary_index(it begin, it end, func f) {
    auto count = 0u;
    for (; begin != end; ++begin) {
        if (f(begin->boundary_type)) {
            begin->boundary_index = count++;
        }
    }
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename It, typename Func>
cl::Buffer init_buffer(It begin,
                       It end,
                       const cl::Context& context,
                       Func func) {
    const auto num_indices = count_boundary_type(begin, end, func);
    if (!num_indices) {
        throw std::runtime_error("no boundaries");
    }
    set_boundary_index(begin, end, func);
    return cl::Buffer{context, CL_MEM_READ_WRITE, sizeof(T) * num_indices};
}

//  mmmmmm beautiful
boundary_index_data compute_boundary_index_data(
        const cl::Device& device,
        const scene_buffers& buffers,
        const mesh_descriptor& descriptor,
        util::aligned::vector<condensed_node>& nodes) {
    //  load up buffers
    auto index_buffer_1 =
            init_buffer<boundary_index_array_1>(nodes.begin(),
                                                nodes.end(),
                                                buffers.get_context(),
                                                is_1d_boundary_or_reentrant);
    auto index_buffer_2 = init_buffer<boundary_index_array_2>(
            nodes.begin(), nodes.end(), buffers.get_context(), is_boundary<2>);
    auto index_buffer_3 = init_buffer<boundary_index_array_3>(
            nodes.begin(), nodes.end(), buffers.get_context(), is_boundary<3>);

    //  load the nodes vector to a cl buffer
    const auto nodes_buffer =
            load_to_buffer(buffers.get_context(), nodes, true);

    //  fire up the program
    const boundary_coefficient_program program{
            compute_context{buffers.get_context(), device}};

    //  create a queue to make sure the cl stuff gets ordered properly
    cl::CommandQueue queue{buffers.get_context(), device};

    //  all our programs use the same size/queue, which can be set up here
    const auto enqueue = [&] {
        return cl::EnqueueArgs{queue, cl::NDRange{nodes.size()}};
    };

    //  run the kernels to compute boundary indices

    auto ret_1 = [&] {
        auto kernel = program.get_boundary_coefficient_finder_1d_kernel();
        kernel(enqueue(),
               nodes_buffer,
               descriptor,
               index_buffer_1,
               buffers.get_voxel_index_buffer(),
               buffers.get_global_aabb(),
               buffers.get_side(),
               buffers.get_triangles_buffer(),
               buffers.get_triangles_buffer().getInfo<CL_MEM_SIZE>() /
                       sizeof(triangle),
               buffers.get_vertices_buffer());
        const auto out =
                read_from_buffer<boundary_index_array_1>(queue, index_buffer_1);

        const auto num_surfaces_1d =
                count_boundary_type(nodes.begin(), nodes.end(), is_boundary<1>);

        util::aligned::vector<boundary_index_array_1> ret;
        ret.reserve(num_surfaces_1d);

        //  we need to remove reentrant nodes from these results
        //  i am dead inside and idk what <algorithm> this is
        for (const auto& i : nodes) {
            if (is_boundary<1>(i.boundary_type)) {
                ret.emplace_back(out[i.boundary_index]);
            }
        }
        return ret;
    }();

    auto ret_2 = [&] {
        auto kernel = program.get_boundary_coefficient_finder_2d_kernel();
        kernel(enqueue(),
               nodes_buffer,
               descriptor,
               index_buffer_2,
               index_buffer_1);
        return read_from_buffer<boundary_index_array_2>(queue, index_buffer_2);
    }();

    auto ret_3 = [&] {
        auto kernel = program.get_boundary_coefficient_finder_3d_kernel();
        kernel(enqueue(),
               nodes_buffer,
               descriptor,
               index_buffer_3,
               index_buffer_1);
        return read_from_buffer<boundary_index_array_3>(queue, index_buffer_3);
    }();

    //  finally, update node boundary indices so that the 1d indices point only
    //  to boundaries and not to reentrant nodes
    set_boundary_index(nodes.begin(), nodes.end(), is_boundary<1>);

    return {std::move(ret_1), std::move(ret_2), std::move(ret_3)};
}

}  // namespace waveguide
