#pragma once

#include "raytracer/image_source/finder.h"
#include "raytracer/image_source/reflection_path_builder.h"
#include "raytracer/postprocess.h"
#include "raytracer/reflector.h"

namespace raytracer {
namespace image_source {

template <typename It>	///	iterator over ray directions
aligned::vector<float> run(It begin,
                           It end,
                           const compute_context& cc,
                           const voxelised_scene_data& voxelised,
                           const glm::vec3& source,
                           const glm::vec3& receiver,
                           float speed_of_sound,
                           float acoustic_impedance,
                           float sample_rate) {
    const auto reflection_depth{raytracer::compute_optimum_reflection_number(
            voxelised.get_scene_data())};

    const scene_buffers buffers{cc.context, voxelised};
    raytracer::reflector ref{
            cc,
            receiver,
            raytracer::get_rays_from_directions(begin, end, source),
            speed_of_sound};

    //  this will collect the first reflections, to a specified depth,
    //  and use them to find unique image-source paths
    raytracer::image_source::finder img{};
    raytracer::image_source::reflection_path_builder builder{
            static_cast<size_t>(std::distance(begin, end))};

    //  run the simulation proper

    //  up until the max reflection depth
    for (auto i{0u}; i != reflection_depth; ++i) {
        //  get a single step of the reflections
        const auto reflections{ref.run_step(buffers)};

        //  find diffuse impulses for these reflections
        builder.push(reflections);
    }

    img.push(builder.get_data());

    const raytracer::image_source::intensity_calculator calculator{
            receiver, voxelised, static_cast<float>(speed_of_sound)};
    const auto img_src_results{image_source::postprocess(
            img, source, receiver, voxelised, speed_of_sound, calculator)};

    return mixdown(raytracer::convert_to_histogram(img_src_results.begin(),
                                                   img_src_results.end(),
                                                   sample_rate,
                                                   acoustic_impedance,
                                                   20));
}

}  // namespace image_source
}  // namespace raytracer
