#include "raytracer/construct_impulse.h"
#include "raytracer/diffuse.h"
#include "raytracer/image_source.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflector.h"

#include "common/nan_checking.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxelised_scene_data.h"

namespace raytracer {

std::experimental::optional<impulse> get_direct_impulse(
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data& scene_data,
        double speed_of_sound) {
    if (source == receiver) {
        return std::experimental::nullopt;
    }

    const auto source_to_receiver{receiver - source};
    const auto source_to_receiver_length{glm::length(source_to_receiver)};
    const auto direction{glm::normalize(source_to_receiver)};
    const geo::ray to_receiver{source, direction};

    const auto intersection{intersects(scene_data, to_receiver)};

    if (!intersection ||
        (intersection && intersection->inter.t > source_to_receiver_length)) {
        return construct_impulse(make_volume_type(1),
                                 source,
                                 source_to_receiver_length,
                                 speed_of_sound);
    }

    return std::experimental::nullopt;
}

std::experimental::optional<results> run(const compute_context& cc,
                                         const voxelised_scene_data& scene_data,
                                         double speed_of_sound,
                                         const glm::vec3& source,
                                         const glm::vec3& receiver,
                                         aligned::vector<glm::vec3> directions,
                                         size_t reflection_depth,
                                         size_t image_source_depth,
                                         const std::atomic_bool& keep_going,
                                         const per_step_callback& callback) {
    if (reflection_depth < image_source_depth) {
        throw std::runtime_error(
                "can't do image-source deeper than the max reflection depth");
    }

    //  set up all the rendering context stuff

    //  load the scene into device memory
    const scene_buffers buffers{cc.context, scene_data};

    //  this is the object that generates first-pass reflections
    reflector ref{cc,
                  receiver,
                  get_rays_from_directions(source, directions),
                  speed_of_sound};

    //  this will collect the first reflections, to a specified depth,
    //  and use them to find unique image-source paths
    image_source_finder img{directions.size(), image_source_depth};

    //  this will incrementally process diffuse responses
    diffuse_finder dif{cc,
                       source,
                       receiver,
                       air_coefficient,
                       speed_of_sound,
                       directions.size(),
                       reflection_depth};

    //  run the simulation proper

    //  up until the max reflection depth
    for (auto i = 0u; i != reflection_depth; ++i) {
        //  if the user cancelled, return an empty result
        if (!keep_going) {
            return std::experimental::nullopt;
        }

        //  get a single step of the reflections
        const auto reflections{ref.run_step(buffers)};

        //  check reflection kernel output
        for (const auto& ref : reflections) {
            throw_if_suspicious(ref.position);
            throw_if_suspicious(ref.direction);
        }

        //  find diffuse impulses for these reflections
        dif.push(reflections, buffers);
        img.push(reflections);

        //  we did a step!
        callback(i);
    }

    return results{
            get_direct_impulse(source, receiver, scene_data, speed_of_sound),
            img.get_results(source, receiver, scene_data, speed_of_sound),
            std::move(dif.get_results()),
            receiver,
            speed_of_sound};
}

}  // namespace raytracer
