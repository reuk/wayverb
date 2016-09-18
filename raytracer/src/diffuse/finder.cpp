#include "raytracer/diffuse/finder.h"

#include "common/map_to_vector.h"
#include "common/nan_checking.h"
#include "common/stl_wrappers.h"

#include <experimental/optional>

namespace raytracer {
namespace diffuse {

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

finder::finder(const compute_context& cc,
               const glm::vec3& source,
               const glm::vec3& receiver,
               double speed_of_sound,
               size_t rays,
               size_t depth)
        : cc(cc)
        , queue(cc.context, cc.device)
        , kernel(program{cc, speed_of_sound}.get_kernel())
        , receiver(to_cl_float3(receiver))
        , rays(rays)
        , reflections_buffer(
                  cc.context, CL_MEM_READ_WRITE, sizeof(reflection) * rays)
        , diffuse_path_buffer(load_to_buffer(
                  cc.context,
                  aligned::vector<diffuse_path_info>(
                          rays,
                          diffuse_path_info{make_volume_type(1.0 / rays),
                                            to_cl_float3(source),
                                            0}),
                  false))
        , impulse_buffer(cc.context, CL_MEM_READ_WRITE, sizeof(impulse) * rays)
        , impulse_builder(rays) {}

void finder::push(const aligned::vector<reflection>& reflections,
                  const scene_buffers& buffers) {
    //  copy the current batch of reflections to the device
    cl::copy(queue, reflections.begin(), reflections.end(), reflections_buffer);

    //  get the kernel and run it
    kernel(cl::EnqueueArgs(queue, cl::NDRange(rays)),
           reflections_buffer,
           receiver,
           buffers.get_triangles_buffer(),
           buffers.get_vertices_buffer(),
           buffers.get_surfaces_buffer(),
           diffuse_path_buffer,
           impulse_buffer);

#ifndef NDEBUG
    const auto buf{
            read_from_buffer<diffuse_path_info>(queue, diffuse_path_buffer)};
    for (auto i{0u}; i != buf.size(); ++i) {
        const auto s{buf[i]};
        throw_if_suspicious(s.volume);
        throw_if_suspicious(s.position);
        throw_if_suspicious(s.distance);
    }
#endif

    //  copy impulses out
    auto ret{read_from_buffer<impulse>(queue, impulse_buffer)};

#ifndef NDEBUG
    for (auto i{0u}; i != ret.size(); ++i) {
        const auto s{ret[i]};
        throw_if_suspicious(s.volume);
        throw_if_suspicious(s.position);
        throw_if_suspicious(s.distance);
    }
#endif

    //  maybe a bit slow but w/e
    //  we profile then we burn it down so that something beautiful can rise
    //  from the weird ashes
    aligned::vector<std::experimental::optional<impulse>> no_invalid;
    no_invalid.reserve(rays);
    for (auto& i : ret) {
        no_invalid.emplace_back(
                i.distance ? std::experimental::make_optional<impulse>(
                                     std::move(i))
                           : std::experimental::nullopt);
    }

    impulse_builder.push(map_to_vector(
            ret, [](auto i) -> std::experimental::optional<impulse> {
                return i.distance ? std::experimental::make_optional<impulse>(
                                            std::move(i))
                                  : std::experimental::nullopt;
            }));
}

const aligned::vector<aligned::vector<impulse>>& finder::get_results() const {
    return impulse_builder.get_data();
}

aligned::vector<aligned::vector<impulse>>& finder::get_results() {
    return impulse_builder.get_data();
}

}  // namespace diffuse
}  // namespace raytracer
