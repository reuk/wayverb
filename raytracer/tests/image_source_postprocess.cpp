#include "raytracer/image_source/tree.h"
#include "raytracer/postprocess.h"
#include "raytracer/reflector.h"

#include "common/azimuth_elevation.h"
#include "common/dsp_vector_ops.h"
#include "common/nan_checking.h"
#include "common/scene_data_loader.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/string_builder.h"
#include "common/write_audio_file.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

namespace {

struct named_scene final {
    std::string name;
    scene_data scene;
};

const aligned::vector<named_scene> test_scenes{
        named_scene{"large_box",
                    geo::get_scene_data(
                            geo::box{glm::vec3(0, 0, 0), glm::vec3(4, 3, 6)})},
        named_scene{"small_box",
                    geo::get_scene_data(
                            geo::box{glm::vec3(0, 0, 0), glm::vec3(3, 3, 3)})},
        named_scene{"vault", scene_data_loader{OBJ_PATH}.get_scene_data()}};

constexpr auto speed_of_sound{340.0};
constexpr auto acoustic_impedance{400.0};
constexpr auto bench_rays{1 << 15};
constexpr auto reflection_depth{20};
constexpr auto sample_rate{44100.0};

const glm::vec3 source{1, 2, 1};
const model::receiver_settings receiver{glm::vec3{1, 2, 2}};
const auto directions{get_random_directions(bench_rays)};

}  // namespace

TEST(image_source_postprocess, intensity) {
    const compute_context cc{};

    for (const auto& pair : test_scenes) {
        const voxelised_scene_data voxelised{
                pair.scene,
                5,
                util::padded(pair.scene.get_aabb(), glm::vec3{0.1})};

        const scene_buffers buffers{cc.context, voxelised};

        //  this is the object that generates first-pass reflections
        raytracer::reflector ref{
                cc,
                receiver.position,
                raytracer::get_rays_from_directions(source, directions),
                speed_of_sound};

        //  this will collect the first reflections, to a specified depth,
        //  and use them to find unique image-source paths
        raytracer::image_source::finder img{directions.size(),
                                            reflection_depth};

        //  run the simulation proper

        //  up until the max reflection depth
        for (auto i = 0u; i != reflection_depth; ++i) {
            //  get a single step of the reflections
            const auto reflections{ref.run_step(buffers)};

            //  check reflection kernel output
            for (const auto& ref : reflections) {
                throw_if_suspicious(ref.position);
                throw_if_suspicious(ref.direction);
            }

            //  find diffuse impulses for these reflections
            img.push(reflections);
        }

        //  fetch image source results
        aligned::vector<impulse> img_src_results{};
        const raytracer::image_source::intensity_calculator calculator{
                receiver.position,
                voxelised,
                static_cast<float>(speed_of_sound)};
        img.postprocess(source,
                        receiver.position,
                        voxelised,
                        speed_of_sound,
                        [&](const auto& a, const auto& b) {
                            img_src_results.push_back(calculator(a, b));
                        });

        auto output{raytracer::run_attenuation(cc,
                                               receiver,
                                               img_src_results,
                                               sample_rate,
                                               speed_of_sound,
                                               acoustic_impedance,
                                               20)};

        if (output.size() != 1) {
            throw std::runtime_error("output should contain just one channel");
        }

        normalize(output);

        const auto fname{build_string(pair.name, ".wav")};
        std::cout << "writing " << fname << std::endl;
        snd::write(fname, output, sample_rate, 16);
    }
}
