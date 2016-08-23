#include "raytracer/diffuse.h"

#include "common/geo/box.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "gtest/gtest.h"

constexpr auto speed_of_sound{340.0};

TEST(diffuse, bad_reflections) {
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

    raytracer::diffuse_finder diff{cc,
                                   source,
                                   receiver,
                                   air_coefficient,
                                   speed_of_sound,
                                   bad_reflections.size(),
                                   1};

    diff.push(bad_reflections, buffers);
}