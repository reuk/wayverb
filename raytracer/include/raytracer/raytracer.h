#pragma once

#include "cl_structs.h"
#include "filters.h"
#include "raytracer_program.h"

#include "common/cl_include.h"
#include "common/scene_data.h"
#include "common/sinc.h"

#include <array>
#include <cmath>
#include <map>
#include <numeric>
#include <vector>

/// Sum impulses ocurring at the same (sampled) time and return a vector in
/// which each subsequent item refers to the next sample of an impulse
/// response.
std::vector<std::vector<float>> flatten_impulses(
        const std::vector<AttenuatedImpulse>& impulse, float samplerate);

/// Maps flattenImpulses over a vector of input vectors.
std::vector<std::vector<std::vector<float>>> flatten_impulses(
        const std::vector<std::vector<AttenuatedImpulse>>& impulse,
        float samplerate);

std::vector<float> mixdown(const std::vector<std::vector<float>>& data);
std::vector<std::vector<float>> mixdown(
        const std::vector<std::vector<std::vector<float>>>& data);

/// Filter and mix down each channel of the input data.
/// Optionally, normalize all channels, trim the tail, and scale the amplitude.
std::vector<std::vector<float>> process(
        filter::FilterType filtertype,
        std::vector<std::vector<std::vector<float>>>& data,
        float sr,
        bool do_normalize,
        float lo_cutoff,
        bool do_trim_tail,
        float volumme_scale);

VolumeType attenuation_for_distance(float distance);

/// Recursively check a collection of Impulses for the earliest non-zero time of
/// an impulse.
template <typename T>
inline float find_predelay(const T& ret) {
    return accumulate(ret.begin() + 1,
                      ret.end(),
                      findPredelay(ret.front()),
                      [](auto a, const auto& b) {
                          auto pd = findPredelay(b);
                          if (a == 0) {
                              return pd;
                          }
                          if (pd == 0) {
                              return a;
                          }
                          return std::min(a, pd);
                      });
}

/// The base case of the findPredelay recursion.
template <>
inline float find_predelay(const AttenuatedImpulse& i) {
    return i.time;
}

/// Recursively subtract a time value from the time fields of a collection of
/// Impulses.
template <typename T>
inline void fix_predelay(T& ret, float seconds) {
    for (auto& i : ret) {
        fixPredelay(i, seconds);
    }
}

/// The base case of the fixPredelay recursion.
template <>
inline void fix_predelay(AttenuatedImpulse& ret, float seconds) {
    ret.time = ret.time > seconds ? ret.time - seconds : 0;
}

/// Fixes predelay by finding and then removing predelay.
template <typename T>
inline void fix_predelay(T& ret) {
    auto predelay = findPredelay(ret);
    fixPredelay(ret, predelay);
}

/// Get the number of necessary reflections for a given min amplitude.
int compute_optimum_reflection_number(float min_amp, float max_reflectivity);

/// Raytraces are calculated in relation to a specific microphone position.
/// This is a struct to keep the impulses and mic position together, because
/// you'll probably never need one without the other.
struct RaytracerResults {
    explicit RaytracerResults(
            const std::vector<Impulse> impulses = std::vector<Impulse>(),
            const glm::vec3& c = glm::vec3(),
            const glm::vec3& s = glm::vec3(),
            int rays = 0,
            int reflections = 0)
            : impulses{impulses}
            , mic{c}
            , source{s}
            , rays{rays}
            , reflections{reflections} {
    }

    const std::vector<Impulse> impulses;
    const glm::vec3 mic;
    const glm::vec3 source;
    const int rays;
    const int reflections;
};

class Results final {
public:
    std::map<std::vector<int>, Impulse> image_source;
    std::vector<Impulse> diffuse;
    glm::vec3 mic;
    glm::vec3 source;
    int rays;
    int reflections;

    RaytracerResults get_diffuse() const;
    RaytracerResults get_image_source(bool remove_direct) const;
    RaytracerResults get_all(bool remove_direct) const;
};

std::vector<cl_float3> get_random_directions(int num);

class Raytracer final {
public:
    using kernel_type = decltype(
            std::declval<RaytracerProgram>().get_improved_raytrace_kernel());

    Raytracer(const RaytracerProgram& program);

    using PerStepCallback = std::function<void()>;

    Results run(const CopyableSceneData& scene_data,
                const glm::vec3& micpos,
                const glm::vec3& source,
                int rays,
                int reflections,
                std::atomic_bool& keep_going,
                const PerStepCallback& callback = PerStepCallback());

    Results run(const CopyableSceneData& scene_data,
                const glm::vec3& micpos,
                const glm::vec3& source,
                const std::vector<cl_float3>& directions,
                int reflections,
                std::atomic_bool& keep_going,
                const PerStepCallback& callback = PerStepCallback());

private:
    cl::CommandQueue queue;
    cl::Context context;
    kernel_type kernel;
};

/// Class for parallel HRTF attenuation of raytrace results.
class Hrtf {
public:
    using kernel_type =
            decltype(std::declval<RaytracerProgram>().get_hrtf_kernel());

    class Config final {
    public:
        glm::vec3 facing;
        glm::vec3 up;
    };

    Hrtf(const RaytracerProgram& program);
    virtual ~Hrtf() noexcept = default;

    /// Attenuate some raytrace results.
    /// The outer vector corresponds to separate channels, the inner vector
    /// contains the impulses, each of which has a time and an 8-band volume.
    std::vector<std::vector<AttenuatedImpulse>> attenuate(
            const RaytracerResults& results, const Config& config);
    std::vector<std::vector<AttenuatedImpulse>> attenuate(
            const RaytracerResults& results,
            const glm::vec3& facing,
            const glm::vec3& up);

    virtual const std::array<std::array<std::array<cl_float8, 180>, 360>, 2>&
    get_hrtf_data() const;

private:
    cl::CommandQueue queue;
    kernel_type kernel;
    const cl::Context context;

    std::vector<AttenuatedImpulse> attenuate(
            const cl_float3& mic_pos,
            int channel,
            const cl_float3& facing,
            const cl_float3& up,
            const std::vector<Impulse>& impulses);

    cl::Buffer cl_in;
    cl::Buffer cl_out;

    cl::Buffer cl_hrtf;
};

/// Class for parallel Speaker attenuation of raytrace results.
class Attenuate {
public:
    using kernel_type =
            decltype(std::declval<RaytracerProgram>().get_attenuate_kernel());

    Attenuate(const RaytracerProgram& program);
    virtual ~Attenuate() noexcept = default;

    /// Attenuate some raytrace results.
    /// The outer vector corresponds to separate channels, the inner vector
    /// contains the impulses, each of which has a time and an 8-band volume.
    std::vector<std::vector<AttenuatedImpulse>> attenuate(
            const RaytracerResults& results,
            const std::vector<Speaker>& speakers);

private:
    std::vector<AttenuatedImpulse> attenuate(
            const glm::vec3& mic_pos,
            const Speaker& speaker,
            const std::vector<Impulse>& impulses);
    cl::CommandQueue queue;
    kernel_type kernel;
    const cl::Context context;

    cl::Buffer cl_in;
    cl::Buffer cl_out;
};
