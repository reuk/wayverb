#include "raytracer/diffuse.h"

#include "common/stl_wrappers.h"

namespace raytracer {

//  each reflection contains
//      position
//      specular direction
//      the intersected triangle
//      whether or not to keep going
//      whether or not it is visible from the receiver
//
//  the diffuse finder needs to keep track of
//      specular energy of each ray
//      distance travelled by each ray

diffuse_finder::diffuse_finder(const cl::Context& context,
                               const cl::Device& device,
                               const glm::vec3& source,
                               const glm::vec3& receiver,
                               const volume_type& air_coefficient,
                               double speed_of_sound,
                               size_t rays,
                               size_t depth)
        : context(context)
        , device(device)
        , queue(context, device)
        , kernel(program(context, device, speed_of_sound).get_diffuse_kernel())
        , receiver(to_cl_float3(receiver))
        , air_coefficient(air_coefficient)
        , rays(rays)
        , reflections_buffer(
                  context, CL_MEM_READ_WRITE, sizeof(reflection) * rays)
        , diffuse_path_buffer(load_to_buffer(
                  context,
                  aligned::vector<diffuse_path_info>(
                          rays,
                          diffuse_path_info{make_volume_type(1.0 / rays),
                                            to_cl_float3(source),
                                            0}),
                  false))
        , impulse_buffer(context, CL_MEM_READ_WRITE, sizeof(impulse) * rays)
        , impulse_builder(rays, depth) {}

void diffuse_finder::push(const aligned::vector<reflection>& reflections,
                          const scene_buffers& buffers) {
    auto is_cl_nan = [](auto i) {
        return proc::any_of(i.s, [](auto i) { return std::isnan(i); });
    };

    //  copy the current batch of reflections to the device
    cl::copy(queue, reflections.begin(), reflections.end(), reflections_buffer);

    //  get the kernel and run it
    kernel(cl::EnqueueArgs(queue, cl::NDRange(rays)),
           reflections_buffer,
           receiver,
           air_coefficient,
           buffers.get_triangles_buffer(),
           buffers.get_vertices_buffer(),
           buffers.get_surfaces_buffer(),
           diffuse_path_buffer,
           impulse_buffer);

    //  copy impulses out
    auto ret = read_from_buffer<impulse>(queue, impulse_buffer);

    for (const auto& i : ret) {
        if (is_cl_nan(i.volume)) {
            throw std::runtime_error("nan in diffuse raytracer output");
        }
    }

    //  maybe a bit slow but w/e
    //  we profile then we burn it down so that something beautiful can rise
    //  from the weird ashes
    aligned::vector<std::experimental::optional<impulse>> no_invalid;
    no_invalid.reserve(rays);
    for (auto& i : ret) {
        no_invalid.push_back(
                i.time ? std::experimental::make_optional<impulse>(std::move(i))
                       : std::experimental::nullopt);
    }

    impulse_builder.push(std::move(no_invalid));
}

const aligned::vector<aligned::vector<impulse>>& diffuse_finder::get_results()
        const {
    return impulse_builder.get_data();
}

aligned::vector<aligned::vector<impulse>>& diffuse_finder::get_results() {
    return impulse_builder.get_data();
}

}  // namespace raytracer
