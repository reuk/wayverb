#include "raytracer/raytracer.h"
#include "raytracer/construct_impulse.h"
#include "raytracer/diffuse.h"
#include "raytracer/image_source.h"
#include "raytracer/reflector.h"
#include "raytracer/scene_buffers.h"

namespace raytracer {

std::experimental::optional<Impulse> get_direct_impulse(
        const glm::vec3& source,
        const glm::vec3& receiver,
        const CopyableSceneData& scene_data,
        const VoxelCollection& vox) {
    VoxelCollection::TriangleTraversalCallback callback(scene_data);

    const auto receiver_to_source = source - receiver;
    const auto direction          = glm::normalize(receiver_to_source);
    const geo::Ray to_receiver(source, direction);

    const auto intersection = vox.traverse(to_receiver, callback);

    if (intersection) {
        return construct_impulse(VolumeType{{1, 1, 1, 1, 1, 1, 1, 1}},
                                 source,
                                 glm::length(receiver_to_source));
    }

    return std::experimental::nullopt;
}

raytracer::raytracer(const cl::Context& context, const cl::Device& device)
        : context(context)
        , device(device) {}

std::experimental::optional<results> raytracer::run(
        const CopyableSceneData& scene_data,
        const glm::vec3& source,
        const glm::vec3& receiver,
        size_t rays,
        size_t reflection_depth,
        size_t image_source_depth,
        std::atomic_bool& keep_going,
        const PerStepCallback& callback) {
    assert(image_source_depth < reflection_depth);

    //  set up all the rendering context stuff

    //  create acceleration structure for raytracing
    VoxelCollection vox(scene_data, 4, 0.1);

    //  load the scene into device memory
    scene_buffers scene_buffers(context, device, scene_data, vox);

    //  this is the object that generates first-pass reflections
    reflector reflector(context, device, source, receiver, rays);

    //  this will collect the first reflections, to a specified depth,
    //  and use them to find unique image-source paths
    image_source_finder image_source_finder(rays, image_source_depth);

    //  this will incrementally process diffuse responses
    diffuse_finder diffuse_finder(
            context, device, receiver, air_coefficient, rays, reflection_depth);

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
        callback();
    }

    return results(
            get_direct_impulse(source, receiver, scene_data, vox),
            image_source_finder.get_results(source, receiver, scene_data, vox),
            std::move(diffuse_finder.get_results()),
            receiver);
}

}  // namespace raytracer
