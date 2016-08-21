#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"

#include "common/azimuth_elevation.h"
#include "common/cl_common.h"
#include "common/range.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/string_builder.h"
#include "common/write_audio_file.h"

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

int main() {
    const auto cc{compute_context{}};

    constexpr double speed_of_sound{340.0};
    constexpr double acoustic_impedance{400.0};
    constexpr double sample_rate{44100.0};
    constexpr glm::vec3 source{0, 1, 0};
    constexpr glm::vec3 receiver{0, 1, 1};

    constexpr std::atomic_bool keep_going{true};

    const aligned::vector<std::pair<std::string, surface>> surfaces{
            std::make_pair("0", make_surface(0.99, 0.01)),
            std::make_pair("1", make_surface(0.8, 0.2)),
            std::make_pair("2", make_surface(0.7, 0.3)),
            std::make_pair("3", make_surface(0.5, 0.5)),
            std::make_pair("4", make_surface(0.3, 0.7)),
            std::make_pair("5", make_surface(0.2, 0.8)),
            std::make_pair("6", make_surface(0.01, 0.99))};

    const aligned::vector<std::pair<std::string, std::string>> objects{
            std::make_pair("vault", OBJ_PATH),
            std::make_pair("tunnel", OBJ_PATH_TUNNEL),
            std::make_pair("bedroom", OBJ_PATH_BEDROOM),
            std::make_pair("bad_box", OBJ_PATH_BAD_BOX)};

    for (auto i : objects) {
        for (auto surf : surfaces) {
            scene_data scene{std::get<1>(i)};
            scene.set_surfaces(std::get<1>(surf));
            const voxelised_scene_data voxelised{
                    scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1})};

            const auto results{raytracer::run(cc.get_context(),
                                              cc.get_device(),
                                              voxelised,
                                              speed_of_sound,
                                              source,
                                              receiver,
                                              get_random_directions(100000),
                                              100,
                                              10,
                                              keep_going,
                                              [](auto) {})};

            const aligned::vector<
                    std::pair<std::string, aligned::vector<impulse>>>
                    impulses{std::make_pair(
                                     "img_src",
                                     results->get_impulses(false, true, false)),
                             std::make_pair("diffuse",
                                            results->get_impulses(
                                                    false, false, true))};

            for (const auto& imp : impulses) {
                const auto sig{raytracer::flatten_filter_and_mixdown(
                        std::get<1>(imp), sample_rate, acoustic_impedance)};
                snd::write(build_string(std::get<0>(imp),
                                        "_",
                                        std::get<0>(surf),
                                        "_",
                                        std::get<0>(i),
                                        ".aiff"),
                           {sig},
                           sample_rate,
                           16);
            }
        }
    }
}
