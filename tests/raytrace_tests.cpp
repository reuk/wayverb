#include "raytracer.h"
#include "cl_common.h"
#include "ostream_overloads.h"
#include "progress.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef MAT_PATH
#define MAT_PATH ""
#endif

template <typename T, typename Prog>
auto get_results(const cl::Context& context,
                 const cl::Device& device,
                 cl::CommandQueue& queue,
                 const Prog& prog,
                 const SceneData& scene_data,
                 const std::vector<cl_float3>& directions,
                 int reflections) {
    T raytrace(prog, queue);
    ProgressBar pb(std::cout, reflections);
    return raytrace.run(scene_data,
                        Vec3f(0, 1.75, 0),
                        Vec3f(0, 1.75, 3),
                        directions,
                        reflections,
                        [&pb] { pb += 1; });
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
    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);
    auto raytrace_program = get_program<RayverbProgram>(context, device);

    SceneData scene_data(OBJ_PATH, MAT_PATH);

    auto directions = get_random_directions(bench_rays);

    auto results_1 = get_results<Raytracer>(context,
                                            device,
                                            queue,
                                            raytrace_program,
                                            scene_data,
                                            directions,
                                            bench_reflections);
}
