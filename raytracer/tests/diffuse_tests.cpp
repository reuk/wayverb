#include "raytracer/diffuse/finder.h"

#include "common/geo/box.h"
#include "common/model/receiver_settings.h"
#include "common/scene_data_loader.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

constexpr auto speed_of_sound{340.0};

TEST(diffuse, bad_reflections_box) {
    const geo::box box(glm::vec3(0, 0, 0), glm::vec3(4, 3, 6));
    constexpr glm::vec3 source{1, 2, 1};
    constexpr glm::vec3 receiver{2, 1, 5};
    constexpr auto s = 0.9;
    constexpr auto d = 0.1;
    constexpr auto surface{make_surface(s, d)};

    const compute_context cc{};

    auto scene = geo::get_scene_data(box);
    scene.set_surfaces(surface);
    const voxelised_scene_data voxelised(
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1}));

    const scene_buffers buffers{cc.context, voxelised};

    const aligned::vector<reflection> bad_reflections{
            reflection{cl_float3{{2.66277409, 0.0182733424, 6}},
                       cl_float3{{-0.474790603, 0.544031799, -0.691811323}},
                       10,
                       true,
                       true},
            reflection{cl_float3{{3.34029818, 1.76905692, 6}},
                       cl_float3{{-0.764549553, -0.314452171, -0.562657714}},
                       10,
                       true,
                       true},
            reflection{cl_float3{{4, 2.46449089, 1.54567611}},
                       cl_float3{{-0.431138247, -0.254869401, 0.86554104}},
                       7,
                       true,
                       true},
    };

    raytracer::diffuse::finder diff{
            cc, source, receiver, speed_of_sound, bad_reflections.size(), 1};

    diff.push(bad_reflections, buffers);
}

TEST(diffuse, bad_reflections_vault) {
    constexpr glm::vec3 source{0, 1, 0};
    const model::receiver_settings receiver{glm::vec3{0, 1, 1}};

    const compute_context cc{};

    const scene_data scene{scene_data_loader{OBJ_PATH}.get_scene_data()};
    const voxelised_scene_data voxelised(
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1}));

    const scene_buffers buffers{cc.context, voxelised};

    const aligned::vector<reflection> bad_reflections{
            reflection{cl_float3{{2.29054403, 1.00505638, -1.5}},
                       cl_float3{{-0.682838321, 0.000305294991, 0.730569482}},
                       2906,
                       true,
                       true},
            reflection{cl_float3{{5.28400469, 3.0999999, -3.8193748}},
                       cl_float3{{-0.715907454, -0.277387351, 0.640728354}},
                       2671,
                       true,
                       true},
            reflection{cl_float3{{5.29999971, 2.40043592, -2.991467}},
                       cl_float3{{-0.778293132, -0.199705943, 0.595295966}},
                       2808,
                       true,
                       true},
            reflection{cl_float3{{-1.29793882, 2.44466829, 5.30000019}},
                       cl_float3{{0.270797491, -0.315135896, -0.909592032}},
                       1705,
                       true,
                       true},
    };

    raytracer::diffuse::finder diff{cc,
                                    source,
                                    receiver.position,
                                    speed_of_sound,
                                    bad_reflections.size(),
                                    1};

    diff.push(bad_reflections, buffers);
}
