#include "raytracer/image_source/finder.h"
#include "raytracer/image_source/reflection_path_builder.h"
#include "raytracer/postprocess.h"
#include "raytracer/reflector.h"

#include "common/azimuth_elevation.h"
#include "common/cl/common.h"
#include "common/dsp_vector_ops.h"
#include "common/progress_bar.h"
#include "common/range.h"
#include "common/scene_data_loader.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/string_builder.h"
#include "common/write_audio_file.h"

#include <iostream>

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef OBJ_PATH_TUNNEL
#define OBJ_PATH_TUNNEL ""
#endif

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

#ifndef OBJ_PATH_BAD_BOX
#define OBJ_PATH_BAD_BOX ""
#endif

template <typename T>
void check(const T& i) {
    if (proc::any_of(i, [](auto j) { return std::isnan(j); })) {
        throw std::runtime_error("don't want nans!");
    }

    if (proc::all_of(i, [](auto j) { return !j; })) {
        throw std::runtime_error("all items zero!");
    }
}

void run_single(const compute_context& cc,
                double speed_of_sound,
                double acoustic_impedance,
                double sample_rate,
                const glm::vec3& source,
                const model::receiver_settings& receiver,
                const std::pair<std::string, std::string>& stage,
                const std::pair<std::string, surface>& surf) {
    scene_data scene{scene_data_loader{std::get<1>(stage)}.get_scene_data()};
    scene.set_surfaces(std::get<1>(surf));
    const voxelised_scene_data voxelised{
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1})};
    const scene_buffers buffers{cc.context, voxelised};

    const auto directions{get_random_directions(100000)};

    raytracer::reflector ref{
            cc,
            receiver.position,
            raytracer::get_rays_from_directions(
                    directions.begin(), directions.end(), source),
            speed_of_sound};

    //  TODO how to calculate reflection depth properly?
    const auto reflection_depth{20};

    //  this will collect the first reflections, to a specified depth,
    //  and use them to find unique image-source paths
    raytracer::image_source::finder img{};
    raytracer::image_source::reflection_path_builder builder{directions.size()};

    //  run the simulation proper

    //  up until the max reflection depth
    for (auto i = 0u; i != reflection_depth; ++i) {
        //  get a single step of the reflections
        const auto reflections{ref.run_step(buffers)};

        //  find diffuse impulses for these reflections
        builder.push(reflections);
    }

    img.push(builder.get_data());

    aligned::vector<impulse> img_src_results{};
    const raytracer::image_source::intensity_calculator calculator{
            receiver.position, voxelised, static_cast<float>(speed_of_sound)};
    img.postprocess(source,
                    receiver.position,
                    voxelised,
                    speed_of_sound,
                    [&](const auto& a, const auto& b) {
                        img_src_results.push_back(calculator(a, b));
                    });

    auto sig{mixdown(raytracer::convert_to_histogram(img_src_results.begin(),
                                                     img_src_results.end(),
                                                     sample_rate,
                                                     acoustic_impedance,
                                                     20))};

    check(sig);
    normalize(sig);
    mul(sig, 1.0f - std::numeric_limits<float>::epsilon());

    snd::write(build_string(std::get<0>(stage), "_", std::get<0>(surf), ".wav"),
               {sig},
               sample_rate,
               32);
}

int main() {
    const compute_context cc{};

    constexpr glm::vec3 source{0, 1, 0};
    const model::receiver_settings receiver{glm::vec3{0, 1, 1}};

    const aligned::vector<std::pair<std::string, surface>> surfaces{
            std::make_pair("0", make_surface(0.001, 0.001)),
            std::make_pair("1", make_surface(0.01, 0.001)),
            std::make_pair("2", make_surface(0.1, 0.001))};

    const aligned::vector<std::pair<std::string, std::string>> objects{
            std::make_pair("vault", OBJ_PATH),
            std::make_pair("tunnel", OBJ_PATH_TUNNEL),
            std::make_pair("bedroom", OBJ_PATH_BEDROOM),
            std::make_pair("bad_box", OBJ_PATH_BAD_BOX)};

    const auto sample_rate{44100};

    for (auto stage : objects) {
        for (auto surf : surfaces) {
            run_single(
                    cc, 340, 400, sample_rate, source, receiver, stage, surf);
        }
    }

    /*
        run_single(cc,
                   340,
                   400,
                   44100,
                   source,
                   receiver
                   objects[0],
                   surfaces[1]);
    */
}
