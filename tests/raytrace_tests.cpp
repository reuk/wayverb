#include "rayverb.h"
#include "cl_common.h"

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
                 const std::vector<cl_float3>& directions) {
    T raytrace(prog, queue);
    return raytrace.run(
        scene_data, Vec3f(0, 1.75, 0), Vec3f(0, 1.75, 3), directions, 128);
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

template <typename T>
bool operator==(const std::vector<T>& a, const std::vector<T>& b) {
    if (a.size() == b.size()) {
        for (auto i = 0u; i != a.size(); ++i) {
            if (!(a[i] == b[i]))
                return false;
        }
    }

    return true;
}

TEST(improved, improved) {
    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);
    auto raytrace_program = get_program<RayverbProgram>(context, device);

    SceneData scene_data(OBJ_PATH, MAT_PATH);

    auto rays = 1 << 12;
    auto directions = get_random_directions(rays);

    auto results_0 = get_results<Raytrace>(
        context, device, queue, raytrace_program, scene_data, directions);
    auto results_1 = get_results<ImprovedRaytrace>(
        context, device, queue, raytrace_program, scene_data, directions);

    results_0.diffuse.resize(rays);
    results_1.diffuse.resize(rays);

    //  TODO
    //  results_0 seems to have image-source contributions which don't start
    //  with '0'
    //
    //  How is this possible? Is this correct?

    for (auto i = 0u; i != results_0.diffuse.size(); ++i) {
        const auto& a = results_0.diffuse[i];
        const auto& b = results_1.diffuse[i];
        ASSERT_EQ(a.volume, b.volume) << "it: " << i;
        ASSERT_EQ(a.position, b.position) << "it: " << i;
        ASSERT_EQ(a.time, b.time) << "it: " << i;
    }
}
