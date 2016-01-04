#pragma once

#include "filters.h"
#include "cl_structs.h"
#include "rayverb_program.h"

#include "config.h"
#include "scene_data.h"
#include "filters.h"
#include "sinc.h"

#include "rapidjson/document.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <vector>
#include <cmath>
#include <numeric>
#include <iostream>
#include <array>
#include <map>

/// Sum impulses ocurring at the same (sampled) time and return a vector in
/// which each subsequent item refers to the next sample of an impulse
/// response.
std::vector<std::vector<float>> flattenImpulses(
    const std::vector<AttenuatedImpulse>& impulse, float samplerate);

/// Maps flattenImpulses over a vector of input vectors.
std::vector<std::vector<std::vector<float>>> flattenImpulses(
    const std::vector<std::vector<AttenuatedImpulse>>& impulse,
    float samplerate);

/// Filter and mix down each channel of the input data.
/// Optionally, normalize all channels, trim the tail, and scale the amplitude.
std::vector<std::vector<float>> process(
    FilterType filtertype,
    std::vector<std::vector<std::vector<float>>>& data,
    float sr,
    bool do_normalize,
    float lo_cutoff,
    bool do_trim_tail,
    float volumme_scale);

/// Recursively check a collection of Impulses for the earliest non-zero time of
/// an impulse.
template <typename T>
inline float findPredelay(const T& ret) {
    return accumulate(ret.begin() + 1,
                      ret.end(),
                      findPredelay(ret.front()),
                      [](auto a, const auto& b) {
                          auto pd = findPredelay(b);
                          if (a == 0)
                              return pd;
                          if (pd == 0)
                              return a;
                          return std::min(a, pd);
                      });
}

/// The base case of the findPredelay recursion.
template <>
inline float findPredelay(const AttenuatedImpulse& i) {
    return i.time;
}

/// Recursively subtract a time value from the time fields of a collection of
/// Impulses.
template <typename T>
inline void fixPredelay(T& ret, float seconds) {
    for (auto&& i : ret)
        fixPredelay(i, seconds);
}

/// The base case of the fixPredelay recursion.
template <>
inline void fixPredelay(AttenuatedImpulse& ret, float seconds) {
    ret.time = ret.time > seconds ? ret.time - seconds : 0;
}

/// Fixes predelay by finding and then removing predelay.
template <typename T>
inline void fixPredelay(T& ret) {
    auto predelay = findPredelay(ret);
    fixPredelay(ret, predelay);
}

/// Raytraces are calculated in relation to a specific microphone position.
/// This is a struct to keep the impulses and mic position together, because
/// you'll probably never need one without the other.
struct RaytracerResults {
    RaytracerResults(const std::vector<Impulse> impulses=std::vector<Impulse>(), const Vec3f& c=Vec3f())
            : impulses{impulses}
            , mic{c} {
    }

    std::vector<Impulse> impulses;
    Vec3f mic;
};

class Raytrace {
public:
    using kernel_type =
        decltype(std::declval<RayverbProgram>().get_raytrace_kernel());

    class Results final {
    public:
        std::map<std::vector<unsigned long>, Impulse> image_source;
        std::vector<Impulse> diffuse;
        Vec3f mic_pos;

        RaytracerResults get_diffuse() const;
        RaytracerResults get_image_source(bool remove_direct) const;
        RaytracerResults get_all(bool remove_direct) const;
    };

    Raytrace(const RayverbProgram& program,
             cl::CommandQueue& queue);

    virtual ~Raytrace() noexcept = default;

    Results run(const SceneData & scene_data,
                const Vec3f& micpos,
                const Vec3f& source,
                int rays,
                int reflections);

private:
    cl::CommandQueue& queue;
    RayverbProgram program;
    kernel_type kernel;
};

#if 0
/// An exciting raytracer.
class Raytrace {
public:
    using kernel_type =
        decltype(std::declval<RayverbProgram>().get_raytrace_kernel());

    Raytrace(const RayverbProgram& program,
             cl::CommandQueue& queue,
             unsigned long nreflections,
             const SceneData & sceneData);

    virtual ~Raytrace() noexcept = default;

    /// Run the raytrace with a specific mic, source, and set of directions.
    void raytrace(const cl_float3& micpos,
                  const cl_float3& source,
                  const std::vector<cl_float3>& directions);
    void raytrace(const cl_float3& micpos, const cl_float3& source, int rays);

    /// Get raw, unprocessed diffuse results.
    RaytracerResults getRawDiffuse();

    /// Get raw, unprocessed image-source results.
    RaytracerResults getRawImages(bool removeDirect);

    /// Get all raw, unprocessed results.
    RaytracerResults getAllRaw(bool removeDirect);

private:
    cl::CommandQueue& queue;
    kernel_type kernel;

    SceneData scene_data;

    const unsigned long nreflections;
    const unsigned long ntriangles;

    cl::Buffer cl_directions;
    cl::Buffer cl_triangles;
    cl::Buffer cl_vertices;
    cl::Buffer cl_surfaces;
    cl::Buffer cl_impulses;
    cl::Buffer cl_image_source;
    cl::Buffer cl_image_source_index;

    cl_float3 storedMicpos;

    static const auto RAY_GROUP_SIZE = 4096u;

    std::vector<Impulse> storedDiffuse;
    std::map<std::vector<unsigned long>, Impulse> imageSourceTally;
};
#endif

/// Class for parallel HRTF attenuation of raytrace results.
class Hrtf {
public:
    using kernel_type =
        decltype(std::declval<RayverbProgram>().get_hrtf_kernel());

    class Config final {
    public:
        Vec3f facing;
        Vec3f up;
    };

    Hrtf(const RayverbProgram& program, cl::CommandQueue& queue);
    virtual ~Hrtf() noexcept = default;

    /// Attenuate some raytrace results.
    /// The outer vector corresponds to separate channels, the inner vector
    /// contains the impulses, each of which has a time and an 8-band volume.
    std::vector<std::vector<AttenuatedImpulse>> attenuate(
        const RaytracerResults& results, const Config& config);
    std::vector<std::vector<AttenuatedImpulse>> attenuate(
        const RaytracerResults& results,
        const Vec3f& facing,
        const Vec3f& up);

    virtual const std::array<std::array<std::array<cl_float8, 180>, 360>, 2>&
    getHrtfData() const;

private:
    cl::CommandQueue& queue;
    kernel_type kernel;
    const cl::Context context;

    std::vector<AttenuatedImpulse> attenuate(
        const cl_float3& mic_pos,
        unsigned long channel,
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
        decltype(std::declval<RayverbProgram>().get_attenuate_kernel());

    Attenuate(const RayverbProgram& program, cl::CommandQueue& queue);
    virtual ~Attenuate() noexcept = default;

    /// Attenuate some raytrace results.
    /// The outer vector corresponds to separate channels, the inner vector
    /// contains the impulses, each of which has a time and an 8-band volume.
    std::vector<std::vector<AttenuatedImpulse>> attenuate(
        const RaytracerResults& results, const std::vector<Speaker>& speakers);

private:
    std::vector<AttenuatedImpulse> attenuate(
        const Vec3f& mic_pos,
        const Speaker& speaker,
        const std::vector<Impulse>& impulses);
    cl::CommandQueue& queue;
    kernel_type kernel;
    const cl::Context context;

    cl::Buffer cl_in;
    cl::Buffer cl_out;
};
