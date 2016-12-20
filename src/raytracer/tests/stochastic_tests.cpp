#include "raytracer/stochastic/finder.h"

#include "core/geo/box.h"
#include "core/scene_data_loader.h"
#include "core/spatial_division/voxelised_scene_data.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

using namespace wayverb::raytracer;
using namespace wayverb::core;

TEST(stochastic, bad_reflections_box) {
    const geo::box box{glm::vec3{0, 0, 0}, glm::vec3{4, 3, 6}};
    constexpr glm::vec3 source{1, 2, 1}, receiver{2, 1, 5};
    constexpr auto s = 0.01;
    constexpr auto d = 0.1;
    constexpr auto surface = make_surface<simulation_bands>(s, d);

    const compute_context cc{};

    const auto scene = geo::get_scene_data(box, surface);
    const auto voxelised = make_voxelised_scene_data(scene, 5, 0.1f);

    const scene_buffers buffers{cc.context, voxelised};

    const util::aligned::vector<reflection> bad_reflections{
            reflection{cl_float3{{2.66277409, 0.0182733424, 6}}, 10, 1, 1},
            reflection{cl_float3{{3.34029818, 1.76905692, 6}}, 10, 1, 1},
            reflection{cl_float3{{4, 2.46449089, 1.54567611}}, 7, 1, 1},
    };

    stochastic::finder diff{cc, source, receiver, 1.0f, bad_reflections.size()};

    diff.process(begin(bad_reflections), end(bad_reflections), buffers);
}

TEST(stochastic, bad_reflections_vault) {
    constexpr glm::vec3 source{0, 1, 0}, receiver{0, 1, 1};

    const compute_context cc{};

    const auto scene = scene_with_extracted_surfaces(
            *scene_data_loader{OBJ_PATH}.get_scene_data(),
            util::aligned::unordered_map<std::string,
                                         surface<simulation_bands>>{});
    const auto voxelised = make_voxelised_scene_data(scene, 5, 0.1f);

    const scene_buffers buffers{cc.context, voxelised};

    const util::aligned::vector<reflection> bad_reflections{
            reflection{cl_float3{{2.29054403, 1.00505638, -1.5}}, 2906, 1, 1},
            reflection{
                    cl_float3{{5.28400469, 3.0999999, -3.8193748}}, 2671, 1, 1},
            reflection{
                    cl_float3{{5.29999971, 2.40043592, -2.991467}}, 2808, 1, 1},
            reflection{cl_float3{{-1.29793882, 2.44466829, 5.30000019}},
                       1705,
                       1,
                       1},
    };

    stochastic::finder diff{cc, source, receiver, 1.0f, bad_reflections.size()};

    diff.process(begin(bad_reflections), end(bad_reflections), buffers);
}
