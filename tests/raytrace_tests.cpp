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
                 const std::vector<cl_float3>& directions,
                 int reflections) {
    T raytrace(prog, queue);
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

static constexpr auto bench_reflections = 128;
static constexpr auto bench_rays = 1 << 15;

TEST(raytrace, old) {
    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);

    try {
        auto raytrace_program = get_program<RayverbProgram>(context, device);

        SceneData scene_data(OBJ_PATH, MAT_PATH);

        auto directions = get_random_directions(bench_rays);
        auto results_0 = get_results<Raytrace>(context,
                                               device,
                                               queue,
                                               raytrace_program,
                                               scene_data,
                                               directions,
                                               bench_reflections);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        throw;
    }
}

TEST(raytrace, new) {
    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);
    auto raytrace_program = get_program<RayverbProgram>(context, device);

    SceneData scene_data(OBJ_PATH, MAT_PATH);

    auto directions = get_random_directions(bench_rays);

    auto results_1 = get_results<ImprovedRaytrace>(context,
                                                   device,
                                                   queue,
                                                   raytrace_program,
                                                   scene_data,
                                                   directions,
                                                   bench_reflections);
}

std::ostream& operator<<(std::ostream& strm, const VolumeType& obj) {
    strm << "( ";
    for (auto i : obj.s)
        strm << i << " ";
    return strm << ")";
}

std::ostream& operator<<(std::ostream& strm, const cl_float3& obj) {
    strm << "( ";
    for (auto i : obj.s)
        strm << i << " ";
    return strm << ")";
}

TEST(raytrace, improved) {
    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);
    auto raytrace_program = get_program<RayverbProgram>(context, device);

    SceneData scene_data(OBJ_PATH, MAT_PATH);

    auto directions = get_random_directions(bench_rays);

    auto results_0 = get_results<Raytrace>(context,
                                           device,
                                           queue,
                                           raytrace_program,
                                           scene_data,
                                           directions,
                                           bench_reflections);
    auto results_1 = get_results<ImprovedRaytrace>(context,
                                                   device,
                                                   queue,
                                                   raytrace_program,
                                                   scene_data,
                                                   directions,
                                                   bench_reflections);

    results_0.diffuse.resize(bench_rays * bench_reflections);
    results_1.diffuse.resize(bench_rays * bench_reflections);

    for (auto i = 0u; i != results_0.diffuse.size(); ++i) {
        const auto& a = results_0.diffuse[i];
        const auto& b = results_1.diffuse[i];

        auto ray_n = i / bench_reflections;
        auto ref_n = i % bench_reflections;
        std::stringstream ss;
        ss << "ray: " << ray_n << ", ref: " << ref_n;

        if (!(a.volume == b.volume && a.position == b.position &&
              a.time == b.time)) {
            auto begin_ind = ray_n * bench_reflections;
            for (auto j = begin_ind; j != i + 1; ++j) {
                std::cout << j << ": pos 0: " << results_0.diffuse[j].position
                          << std::endl;
                std::cout << j << ": pos 1: " << results_1.diffuse[j].position
                          << std::endl;
                std::cout << j << ": vol 0: " << results_0.diffuse[j].volume
                          << std::endl;
                std::cout << j << ": vol 1: " << results_1.diffuse[j].volume
                          << std::endl;
            }
        }

        ASSERT_EQ(a.volume, b.volume) << ss.str();
        ASSERT_EQ(a.position, b.position) << ss.str();
        ASSERT_EQ(a.time, b.time) << ss.str();
    }

    for (const auto& i : results_0.image_source) {
        auto it = results_1.image_source.find(i.first);
        ASSERT_NE(results_1.image_source.end(), it);
    }
}
