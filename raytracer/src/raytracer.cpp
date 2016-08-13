#include "raytracer/raytracer.h"
#include "raytracer/construct_impulse.h"
#include "raytracer/diffuse.h"
#include "raytracer/image_source.h"
#include "raytracer/reflector.h"

#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxelised_scene_data.h"

namespace raytracer {

std::experimental::optional<impulse> get_direct_impulse(
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data& scene_data) {
    const auto source_to_receiver = receiver - source;
    const auto source_to_receiver_length = glm::length(source_to_receiver);
    const auto direction = glm::normalize(source_to_receiver);
    const geo::ray to_receiver(source, direction);

    const auto intersection = intersects(scene_data, to_receiver);

    if (!intersection ||
        (intersection && intersection->inter.t > source_to_receiver_length)) {
        return construct_impulse(volume_type{{1, 1, 1, 1, 1, 1, 1, 1}},
                                 source,
                                 source_to_receiver_length);
    }

    return std::experimental::nullopt;
}

std::experimental::optional<results> run(const cl::Context& context,
                                         const cl::Device& device,
                                         const voxelised_scene_data& scene_data,
                                         const glm::vec3& source,
                                         const glm::vec3& receiver,
                                         size_t rays,
                                         size_t reflection_depth,
                                         size_t image_source_depth,
                                         std::atomic_bool& keep_going,
                                         const per_step_callback& callback) {
    if (reflection_depth < image_source_depth) {
        throw std::runtime_error(
                "can't do image-source deeper than the max reflection depth");
    }

    //  set up all the rendering context stuff

    //  load the scene into device memory
    scene_buffers scene_buffers(context, scene_data);

    //  this is the object that generates first-pass reflections
    reflector reflector(context, device, source, receiver, rays);

    //  this will collect the first reflections, to a specified depth,
    //  and use them to find unique image-source paths
    image_source_finder image_source_finder(rays, image_source_depth);

    //  this will incrementally process diffuse responses
    diffuse_finder diffuse_finder(context,
                                  device,
                                  source,
                                  receiver,
                                  air_coefficient,
                                  rays,
                                  reflection_depth);

    //  run the simulation proper

    //  up until the max reflection depth
    for (auto i = 0u; i != reflection_depth; ++i) {
        //  if the user cancelled, return an empty result
        if (!keep_going) {
            return std::experimental::nullopt;
        }

        //  get a single step of the reflections
        const auto reflections = reflector.run_step(scene_buffers);

        //  find diffuse impulses for these reflections
        diffuse_finder.push(reflections, scene_buffers);
        image_source_finder.push(reflections);

        //  we did a step!
        callback(i);
    }

    return results(
            get_direct_impulse(source, receiver, scene_data),
            image_source_finder.get_results(source, receiver, scene_data),
            std::move(diffuse_finder.get_results()),
            receiver);
}

}  // namespace raytracer
