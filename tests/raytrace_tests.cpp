#include "raytracer.h"
#include "raytracer_config.h"
#include "cl_common.h"
#include "ostream_overloads.h"
#include "progress.h"
#include "boundaries.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef MAT_PATH
#define MAT_PATH ""
#endif

template <typename T, typename Prog>
auto get_results(ComputeContext& context,
                 const Prog& prog,
                 const SceneData& scene_data,
                 const std::vector<cl_float3>& directions,
                 int reflections) {
    T raytrace(prog, context.queue);
    return raytrace.run(scene_data,
                        Vec3f(0, 1.75, 0),
                        Vec3f(0, 1.75, 3),
                        directions,
                        reflections);
}

#define USE_EPSILON

#ifdef USE_EPSILON
const auto EPSILON = 0.00001;
#endif

bool operator==(const cl_float3& a, const cl_float3& b) {
    for (auto i = 0u; i != 3; ++i) {
#ifdef USE_EPSILON
        if (fabs(a.s[i] - b.s[i]) > EPSILON)
            return false;
#else
        if (a.s[i] != b.s[i])
            return false;
#endif
    }
    return true;
}

bool operator==(const VolumeType& a, const VolumeType& b) {
    for (auto i = 0u; i != sizeof(a) / sizeof(a.s[0]); ++i) {
#ifdef USE_EPSILON
        if (fabs(a.s[i] - b.s[i]) > EPSILON)
            return false;
#else
        if (a.s[i] != b.s[i])
            return false;
#endif
    }
    return true;
}

bool operator==(const Impulse& a, const Impulse& b) {
    if (!(a.volume == b.volume))
        return false;
    if (!(a.position == b.position))
        return false;
    if (!(a.time == b.time))
        return false;

    return true;
}

static constexpr auto bench_reflections = 128;
static constexpr auto bench_rays = 1 << 15;

TEST(raytrace, new) {
    ComputeContext context;
    auto raytrace_program = get_program<RaytracerProgram>(context);

    SceneData scene_data(OBJ_PATH, MAT_PATH);

    auto directions = get_random_directions(bench_rays);

    auto results_1 = get_results<Raytracer>(
        context, raytrace_program, scene_data, directions, bench_reflections);
}

TEST(raytrace, image_source) {
    ComputeContext context;
    auto raytrace_program = get_program<RaytracerProgram>(context);

    CuboidBoundary boundary(Vec3f(0, 0, 0), Vec3f(4, 3, 6));

    RaytracerConfig config;
    config.get_source() = Vec3f(2, 1, 1);
    config.get_mic() = Vec3f(2, 1, 5);

    Raytracer raytracer(raytrace_program, context.queue);

    auto results = raytracer.run(boundary.get_scene_data(),
                                 config.get_mic(),
                                 config.get_source(),
                                 config.get_rays(),
                                 config.get_impulses());

    Attenuate attenuator(raytrace_program, context.queue);
    Speaker speaker{};
    auto output =
        attenuator.attenuate(results.get_image_source(false), {speaker});

    //  TODO compare image source results to results found analytically or using
    //  some alternate method
}
