#pragma once

#include "filters.h"
#include "clstructs.h"

#include "rtaudiocommon/filters.h"
#include "rtaudiocommon/sinc.h"

#include "rapidjson/document.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <vector>
#include <cmath>
#include <numeric>
#include <iostream>
#include <array>
#include <map>

//#define DIAGNOSTIC

/// Sum impulses ocurring at the same (sampled) time and return a vector in
/// which each subsequent item refers to the next sample of an impulse
/// response.
std::vector <std::vector <float>> flattenImpulses
(   const std::vector <AttenuatedImpulse> & impulse
,   float samplerate
);

/// Maps flattenImpulses over a vector of input vectors.
std::vector <std::vector <std::vector <float>>> flattenImpulses
(   const std::vector <std::vector <AttenuatedImpulse>> & impulse
,   float samplerate
);

/// Filter and mix down each channel of the input data.
/// Optionally, normalize all channels, trim the tail, and scale the amplitude.
std::vector <std::vector <float>> process
(   FilterType filtertype
,   std::vector <std::vector <std::vector <float>>> & data
,   float sr
,   bool do_normalize
,   float lo_cutoff
,   bool do_trim_tail
,   float volumme_scale
);

/// Recursively check a collection of Impulses for the earliest non-zero time of
/// an impulse.
template <typename T>
inline float findPredelay (const T & ret)
{
    return accumulate
    (   ret.begin() + 1
    ,   ret.end()
    ,   findPredelay (ret.front())
    ,   [] (auto a, const auto & b)
        {
            auto pd = findPredelay (b);
            if (a == 0)
                return pd;
            if (pd == 0)
                return a;
            return std::min (a, pd);
        }
    );
}

/// The base case of the findPredelay recursion.
template<>
inline float findPredelay (const AttenuatedImpulse & i)
{
    return i.time;
}

/// Recursively subtract a time value from the time fields of a collection of
/// Impulses.
template <typename T>
inline void fixPredelay (T & ret, float seconds)
{
    for (auto && i : ret)
        fixPredelay (i, seconds);
}

/// The base case of the fixPredelay recursion.
template<>
inline void fixPredelay (AttenuatedImpulse & ret, float seconds)
{
    ret.time = ret.time > seconds ? ret.time - seconds : 0;
}

/// Fixes predelay by finding and then removing predelay.
template <typename T>
inline void fixPredelay (T & ret)
{
    auto predelay = findPredelay (ret);
    fixPredelay (ret, predelay);
}

/// Class wrapping an OpenCL context.
class ContextProvider
{
public:
    ContextProvider();

    cl::Context cl_context;
};

/// Builds a specific kernel and sets up a CommandQueue on an OpenCL context.
class KernelLoader: public ContextProvider
{
public:
    KernelLoader();
    KernelLoader(bool verbose);

    cl::Program cl_program;
    cl::CommandQueue queue;
    static const std::string KERNEL_STRING;
};

/// Raytraces are calculated in relation to a specific microphone position.
/// This is a struct to keep the impulses and mic position together, because
/// you'll probably never need one without the other.
struct RaytracerResults
{
    RaytracerResults() {}
    RaytracerResults (const std::vector <Impulse> impulses, const cl_float3 & c)
    :   impulses (impulses)
    ,   mic (c)
    {}

    std::vector <Impulse> impulses;
    cl_float3 mic;
};

/// An exciting raytracer.
class Raytracer: public KernelLoader
{
public:

    /// If you don't want to use the built-in object loader, you can
    /// initialise a raytracer with your own geometry here.
    Raytracer
    (   unsigned long nreflections
    ,   std::vector <Triangle> & triangles
    ,   std::vector <cl_float3> & vertices
    ,   std::vector <Surface> & surfaces
    ,   bool verbose
    );

    /// Load a 3d model and materials from files.
    Raytracer
    (   unsigned long nreflections
    ,   const std::string & objpath
    ,   const std::string & materialFileName
    ,   bool verbose
    );

    /// Run the raytrace with a specific mic, source, and set of directions.
    void raytrace
    (   const cl_float3 & micpos
    ,   const cl_float3 & source
    ,   const std::vector <cl_float3> & directions
    ,   bool verbose
    );

    /// Get raw, unprocessed diffuse results.
    RaytracerResults getRawDiffuse();

    /// Get raw, unprocessed image-source results.
    RaytracerResults getRawImages (bool removeDirect);

    /// Get all raw, unprocessed results.
    RaytracerResults getAllRaw (bool removeDirect);

private:
    const unsigned long nreflections;
    const unsigned long ntriangles;

    cl::Buffer cl_directions;
    cl::Buffer cl_triangles;
    cl::Buffer cl_vertices;
    cl::Buffer cl_surfaces;
    cl::Buffer cl_impulses;
    cl::Buffer cl_image_source;
    cl::Buffer cl_image_source_index;

    std::pair <cl_float3, cl_float3> bounds;

    cl_float3 storedMicpos;

    struct SceneData;

    Raytracer
    (   unsigned long nreflections
    ,   SceneData sceneData
    ,   bool verbose
    );

    static const unsigned int RAY_GROUP_SIZE = 4096;

    decltype
    (   cl::make_kernel
        <   cl::Buffer
        ,   cl_float3
        ,   cl::Buffer
        ,   cl_ulong
        ,   cl::Buffer
        ,   cl_float3
        ,   cl::Buffer
        ,   cl::Buffer
        ,   cl::Buffer
        ,   cl::Buffer
        ,   cl_ulong
        ,   VolumeType
        > (cl_program, "raytrace")
    ) raytrace_kernel;

    std::vector <Impulse> storedDiffuse;
    std::map <std::vector <unsigned long>, Impulse> imageSourceTally;
};

/// HRTF parameters.
struct HrtfConfig
{
    cl_float3 facing;
    cl_float3 up;
};

/// An attenuator is just a KernelLoader with some extra buffers.
struct Attenuator: public KernelLoader
{
    cl::Buffer cl_in;
    cl::Buffer cl_out;
};

/// Class for parallel HRTF attenuation of raytrace results.
class HrtfAttenuator: public Attenuator
{
public:
    HrtfAttenuator();

    /// Attenuate some raytrace results.
    /// The outer vector corresponds to separate channels, the inner vector
    /// contains the impulses, each of which has a time and an 8-band volume.
    std::vector <std::vector <AttenuatedImpulse>> attenuate
    (   const RaytracerResults & results
    ,   const HrtfConfig & config
    );
    std::vector <std::vector <AttenuatedImpulse>> attenuate
    (   const RaytracerResults & results
    ,   const cl_float3 & facing
    ,   const cl_float3 & up
    );

    virtual const std::array <std::array <std::array <cl_float8, 180>, 360>, 2> & getHrtfData() const;
private:
    static const std::array <std::array <std::array <cl_float8, 180>, 360>, 2> HRTF_DATA;
    std::vector <AttenuatedImpulse> attenuate
    (   const cl_float3 & mic_pos
    ,   unsigned long channel
    ,   const cl_float3 & facing
    ,   const cl_float3 & up
    ,   const std::vector <Impulse> & impulses
    );

    cl::Buffer cl_hrtf;

    decltype
    (   cl::make_kernel
        <   cl_float3
        ,   cl::Buffer
        ,   cl::Buffer
        ,   cl::Buffer
        ,   cl_float3
        ,   cl_float3
        ,   cl_ulong
        > (cl_program, "hrtf")
    ) attenuate_kernel;
};

/// Class for parallel Speaker attenuation of raytrace results.
class SpeakerAttenuator: public Attenuator
{
public:
    SpeakerAttenuator();

    /// Attenuate some raytrace results.
    /// The outer vector corresponds to separate channels, the inner vector
    /// contains the impulses, each of which has a time and an 8-band volume.
    std::vector <std::vector <AttenuatedImpulse>> attenuate
    (   const RaytracerResults & results
    ,   const std::vector <Speaker> & speakers
    );
private:
    std::vector <AttenuatedImpulse> attenuate
    (   const cl_float3 & mic_pos
    ,   const Speaker & speaker
    ,   const std::vector <Impulse> & impulses
    );
    decltype
    (   cl::make_kernel
        <   cl_float3
        ,   cl::Buffer
        ,   cl::Buffer
        ,   Speaker
        > (cl_program, "attenuate")
    ) attenuate_kernel;
};

/// Try to open and parse a json file.
void attemptJsonParse
(   const std::string & fname
,   rapidjson::Document & doc
);
