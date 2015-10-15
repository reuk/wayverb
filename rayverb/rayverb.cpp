#include "rayverb.h"
#include "filters.h"
#include "config.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/error/en.h"
#include "rapidjson/document.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <cmath>
#include <numeric>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace rapidjson;

inline cl_float3 fromAIVec (const aiVector3D & v)
{
    return (cl_float3) {{v.x, v.y, v.z, 0}};
}

vector <vector <vector <float>>> flattenImpulses
(   const vector <vector <AttenuatedImpulse>> & attenuated
,   float samplerate
)
{
    vector <vector <vector <float>>> flattened (attenuated.size());
    transform
    (   begin (attenuated)
    ,   end (attenuated)
    ,   begin (flattened)
    ,   [samplerate] (const auto & i)
        {
            return flattenImpulses (i, samplerate);
        }
    );
    return flattened;
}

/// Turn a collection of AttenuatedImpulses into a vector of 8 vectors, where
/// each of the 8 vectors represent sample values in a different frequency band.
vector <vector <float>> flattenImpulses
(   const vector <AttenuatedImpulse> & impulse
,   float samplerate
)
{
    // Find the index of the final sample based on time and samplerate
    float maxtime = 0;
    for (const auto & i : impulse)
        maxtime = max (maxtime, i.time);
    const auto MAX_SAMPLE = round (maxtime * samplerate) + 1;

    //  Create somewhere to store the results.
    vector <vector <float>> flattened
    (   sizeof (VolumeType) / sizeof (float)
    ,   vector <float> (MAX_SAMPLE, 0)
    );

    //  For each impulse, calculate its index, then add the impulse's volumes
    //  to the volumes already in the output array.
    for (const auto & i : impulse)
    {
        const auto SAMPLE = round (i.time * samplerate);
        for (auto j = 0u; j != flattened.size(); ++j)
        {
            flattened [j] [SAMPLE] += i.volume.s [j];
        }
    }

    return flattened;
}

/// Sum a collection of vectors of the same length into a single vector
vector <float> mixdown (const vector <vector <float>> & data)
{
    vector <float> ret (data.front().size(), 0);
    for (auto && i : data)
        transform
        (   ret.begin()
        ,   ret.end()
        ,   i.begin()
        ,   ret.begin()
        ,   plus <float>()
        );
    return ret;
}

/// Find the index of the last sample with an amplitude of minVol or higher,
/// then resize the vectors down to this length.
void trimTail (vector <vector <float>> & audioChannels, float minVol)
{
    // Find last index of required amplitude or greater.
    auto len = accumulate
    (   audioChannels.begin()
    ,   audioChannels.end()
    ,   0
    ,   [minVol] (auto current, const auto & i)
        {
            return max
            (   current
            ,   distance
                (   i.begin()
                ,   find_if
                    (   i.rbegin()
                    ,   i.rend()
                    ,   [minVol] (auto j) {return abs (j) >= minVol;}
                    ).base()
                ) - 1
            );
        }
    );

    // Resize.
    for (auto && i : audioChannels)
        i.resize (len);
}

/// Collects together all the post-processing steps.
vector <vector <float>> process
(   FilterType filtertype
,   vector <vector <vector <float>>> & data
,   float sr
,   bool do_normalize
,   float lo_cutoff
,   bool do_trim_tail
,   float volume_scale
)
{
    filter (filtertype, data, sr, lo_cutoff);
    vector <vector <float>> ret (data.size());
    transform (data.begin(), data.end(), ret.begin(), mixdown);

    if (do_normalize)
        normalize (ret);

    if (volume_scale != 1)
        mul (ret, volume_scale);

    if (do_trim_tail)
        trimTail (ret, 0.00001);

    return ret;
}

ContextProvider::ContextProvider()
{
    // Set up a GPU context.
    vector <cl::Platform> platform;
    cl::Platform::get (&platform);

    cl_context_properties cps [3] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties) (platform [0]) (),
        0
    };

    cl_context = cl::Context (CL_DEVICE_TYPE_GPU, cps);
}

KernelLoader::KernelLoader()
:   KernelLoader (false)
{

}

KernelLoader::KernelLoader(bool verbose)
:   cl_program (cl_context, KERNEL_STRING, false)
{
    // Grab the final device that the context makes available.
    vector <cl::Device> device = cl_context.getInfo <CL_CONTEXT_DEVICES>();
    vector <cl::Device> used_devices (device.end() - 1, device.end());

    // Build program for this device.
    cl_program.build (used_devices);
    cl::Device used_device = used_devices.front();

    if (verbose)
    {
        cerr
        <<  cl_program.getBuildInfo <CL_PROGRAM_BUILD_LOG> (used_device)
        <<  endl;
    }

    // Set up a queue on this device.
    queue = cl::CommandQueue (cl_context, used_device);
}

/// Call binary operation u on pairs of elements from a and b, where a and b are
/// cl_floatx types.
template <typename T, typename U>
inline T elementwise (const T & a, const T & b, const U & u)
{
    T ret;
    std::transform (std::begin (a.s), std::end (a.s), std::begin (b.s), std::begin (ret.s), u);
    return ret;
}

/// Find the minimum and maximum boundaries of a set of vertices.
pair <cl_float3, cl_float3> getBounds
(   const vector <cl_float3> & vertices
)
{
    return pair <cl_float3, cl_float3>
    (   accumulate
        (   vertices.begin() + 1
        ,   vertices.end()
        ,   vertices.front()
        ,   [] (const auto & a, const auto & b)
            {
                return elementwise (a, b, [] (auto i, auto j)
                    {
                        return min (i, j);
                    }
                );
            }
        )
    ,   accumulate
        (   vertices.begin() + 1
        ,   vertices.end()
        ,   vertices.front()
        ,   [] (const auto & a, const auto & b)
            {
                return elementwise (a, b, [] (auto i, auto j)
                    {
                        return max (i, j);
                    }
                );
            }
        )
    );
}

/// Does a point fall within the cuboid defined by the point pair bounds?
bool inside
(   const pair <cl_float3, cl_float3> & bounds
,   const cl_float3 & point
)
{
    for (auto i = 0u; i != sizeof (cl_float3) / sizeof (float); ++i)
        if (! (bounds.first.s [i] <= point.s [i] && point.s [i] <= bounds.second.s [i]))
            return false;
    return true;
}

/// Reserve graphics memory.
Raytracer::Raytracer
(   unsigned long nreflections
,   vector <Triangle> & triangles
,   vector <cl_float3> & vertices
,   vector <Surface> & surfaces
,   bool verbose
)
:   KernelLoader (verbose)
,   nreflections (nreflections)
,   ntriangles (triangles.size())
,   cl_directions
    (   cl_context
    ,   CL_MEM_READ_WRITE
    ,   RAY_GROUP_SIZE * sizeof (cl_float3)
    )
,   cl_triangles  (cl_context, begin (triangles),  end (triangles),  false)
,   cl_vertices   (cl_context, begin (vertices),   end (vertices),   false)
,   cl_surfaces   (cl_context, begin (surfaces),   end (surfaces),   false)
,   cl_impulses
    (   cl_context
    ,   CL_MEM_READ_WRITE
    ,   RAY_GROUP_SIZE * nreflections * sizeof (Impulse)
    )
,   cl_image_source
    (   cl_context
    ,   CL_MEM_READ_WRITE
    ,   RAY_GROUP_SIZE * NUM_IMAGE_SOURCE * sizeof (Impulse)
    )
,   cl_image_source_index
    (   cl_context
    ,   CL_MEM_READ_WRITE
    ,   RAY_GROUP_SIZE * NUM_IMAGE_SOURCE * sizeof (cl_ulong)
    )
,   bounds (getBounds (vertices))
,   raytrace_kernel
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
    )
{
}

/// Utility class for loading and extracting data from 3d object files.
struct Raytracer::SceneData
{
public:
    SceneData (const string & objpath, const string & materialFileName, bool verbose)
    {
        populate (objpath, materialFileName, verbose);
    }

    map <string, Surface> extractSurfaces (const string & materialFileName)
    {
        Document document;
        attemptJsonParse (materialFileName, document);
        if (! document.IsObject())
            throw runtime_error ("Materials must be stored in a JSON object");

        map <string, Surface> ret;
        for
        (   auto i = document.MemberBegin()
        ;   i != document.MemberEnd()
        ;   ++i
        )
        {
            string name = i->name.GetString();

            Surface surface;
            ValueJsonValidator <Surface> getter (surface);
            getter.run (i->value);
            ret [name] = surface;
        }

        return ret;
    }

    /// Given a scene and a material file, match meshes to materials and extract
    /// faces.
    void populate (const aiScene * scene, const string & materialFileName, bool verbose)
    {
        if (! scene)
            throw runtime_error ("Failed to load object file.");

        Surface defaultSurface = {
            (VolumeType) {{0.92, 0.92, 0.93, 0.93, 0.94, 0.95, 0.95, 0.95}},
            (VolumeType) {{0.50, 0.90, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95}}
        };

        surfaces.push_back (defaultSurface);

        Document document;
        attemptJsonParse (materialFileName, document);
        if (! document.IsObject())
            throw runtime_error ("Materials must be stored in a JSON object");

        auto surfaceMap = extractSurfaces (materialFileName);
        map <string, int> materialIndices;
        for (const auto & i : surfaceMap)
        {
            surfaces.push_back (i.second);
            materialIndices [i.first] = surfaces.size() - 1;
        }

        for (auto i = 0u; i != scene->mNumMeshes; ++i)
        {
            const aiMesh * mesh = scene->mMeshes [i];

            aiString meshName = mesh->mName;
            if (verbose)
                cerr << "Found mesh: " << meshName.C_Str() << endl;
            const aiMaterial * material =
                scene->mMaterials [mesh->mMaterialIndex];

            aiString matName;
            material->Get (AI_MATKEY_NAME, matName);

            unsigned long mat_index = 0;
            auto nameIterator = materialIndices.find (matName.C_Str());
            if (nameIterator != materialIndices.end())
                mat_index = nameIterator->second;

            if (verbose)
            {
                Surface surface = surfaces [mat_index];

                cerr << "    Material name: " << matName.C_Str() << endl;

                cerr << "    Material properties: " << endl;
                cerr << "        specular: ["
                     << surface.specular.s [0] << ", "
                     << surface.specular.s [1] << ", "
                     << surface.specular.s [2] << ", "
                     << surface.specular.s [3] << ", "
                     << surface.specular.s [4] << ", "
                     << surface.specular.s [5] << ", "
                     << surface.specular.s [6] << ", "
                     << surface.specular.s [7] << "]"
                     << endl;
                cerr << "        diffuse: ["
                     << surface.diffuse.s [0] << ", "
                     << surface.diffuse.s [1] << ", "
                     << surface.diffuse.s [2] << ", "
                     << surface.diffuse.s [3] << ", "
                     << surface.diffuse.s [4] << ", "
                     << surface.diffuse.s [5] << ", "
                     << surface.diffuse.s [6] << ", "
                     << surface.diffuse.s [7] << "]"
                     << endl;
            }

            vector <cl_float3> meshVertices (mesh->mNumVertices);

            for (auto j = 0u; j != mesh->mNumVertices; ++j)
            {
                meshVertices [j] = fromAIVec (mesh->mVertices [j]);
            }

            vector <Triangle> meshTriangles (mesh->mNumFaces);

            for (auto j = 0u; j != mesh->mNumFaces; ++j)
            {
                const aiFace face = mesh->mFaces [j];

                meshTriangles [j] = (Triangle) {
                    mat_index,
                    vertices.size() + face.mIndices [0],
                    vertices.size() + face.mIndices [1],
                    vertices.size() + face.mIndices [2]
                };
            }

            vertices.insert
            (   vertices.end()
            ,   begin (meshVertices)
            ,   end (meshVertices)
            );

            triangles.insert
            (   triangles.end()
            ,   begin (meshTriangles)
            ,   end (meshTriangles)
            );
        }

        if (verbose)
        {
            cerr
            <<  "Loaded 3D model with "
            <<  triangles.size()
            <<  " triangles"
            <<  endl;
        }
    }

    void populate (const string & objpath, const string & materialFileName, bool verbose)
    {
        Assimp::Importer importer;
        populate
        (   importer.ReadFile
            (   objpath
            ,   (   aiProcess_Triangulate
                |   aiProcess_GenSmoothNormals
                |   aiProcess_FlipUVs
                )
            )
        ,   materialFileName
        ,   verbose
        );
    }

    bool validSurfaces()
    {
        for (const auto & s : surfaces)
        {
            for (auto i = 0u; i != 3; ++i)
            {
                if
                (   s.specular.s [i] < 0 || 1 < s.specular.s [i]
                ||  s.diffuse.s [i] < 0 || 1 < s.diffuse.s [i]
                )
                    return false;
            }
        }

        return true;
    }

    bool validTriangles()
    {
        for (const auto & t : triangles)
        {
            if
            (   surfaces.size() <= t.surface
            ||  vertices.size() <= t.v0
            ||  vertices.size() <= t.v1
            ||  vertices.size() <= t.v2
            )
                return false;
        }

        return true;
    }

    bool valid()
    {
        if (triangles.empty() || vertices.empty() || surfaces.empty())
            return false;

        return validSurfaces() && validTriangles();
    }

    vector <Triangle> triangles;
    vector <cl_float3> vertices;
    vector <Surface> surfaces;
};

Raytracer::Raytracer
(   unsigned long nreflections
,   const string & objpath
,   const string & materialFileName
,   bool verbose
)
:   Raytracer
(   nreflections
,   SceneData (objpath, materialFileName, verbose)
,   verbose
)
{
}

Raytracer::Raytracer
(   unsigned long nreflections
,   SceneData sceneData
,   bool verbose
)
:   Raytracer
(   nreflections
,   sceneData.triangles
,   sceneData.vertices
,   sceneData.surfaces
,   verbose
)
{
}

void Raytracer::raytrace
(   const cl_float3 & micpos
,   const cl_float3 & source
,   const vector <cl_float3> & directions
,   bool verbose
)
{
    storedMicpos = micpos;

    //  check that mic and source are inside model bounds
    bool micinside = inside (bounds, micpos);
    bool srcinside = inside (bounds, source);
    if (verbose && (! (micinside && srcinside)))
    {
        cerr
        <<  "model bounds: ["
        <<  bounds.first.s [0] << ", "
        <<  bounds.first.s [1] << ", "
        <<  bounds.first.s [2] << "], ["
        <<  bounds.second.s [0] << ", "
        <<  bounds.second.s [1] << ", "
        <<  bounds.second.s [2] << "]"
        <<  endl;

        if (! micinside)
        {
            cerr << "WARNING: microphone position may be outside model" << endl;
            cerr
            <<  "mic position: ["
            <<  micpos.s [0] << ", "
            <<  micpos.s [1] << ", "
            <<  micpos.s [2] << "]"
            <<  endl;
        }

        if (! srcinside)
        {
            cerr << "WARNING: source position may be outside model" << endl;
            cerr
            <<  "src position: ["
            <<  source.s [0] << ", "
            <<  source.s [1] << ", "
            <<  source.s [2] << "]"
            <<  endl;
        }
    }

    imageSourceTally.clear();
    storedDiffuse.resize (directions.size() * nreflections);
    for (auto i = 0u; i != ceil (directions.size() / float (RAY_GROUP_SIZE)); ++i)
    {
        auto b = i * RAY_GROUP_SIZE;
        auto e = min (directions.size(), (i + 1) * RAY_GROUP_SIZE);

        //  copy input to buffer
        cl::copy
        (   queue
        ,   directions.begin() + b
        ,   directions.begin() + e
        ,   cl_directions
        );

        //  zero out impulse storage memory
        vector <Impulse> diffuse
            (RAY_GROUP_SIZE * nreflections, (Impulse) {{{0}}});
        cl::copy (queue, begin (diffuse), end (diffuse), cl_impulses);

        vector <Impulse> image
            (RAY_GROUP_SIZE * NUM_IMAGE_SOURCE, (Impulse) {{{0}}});
        cl::copy (queue, begin (image), end (image), cl_image_source);

        vector <unsigned long> image_source_index
            (RAY_GROUP_SIZE * NUM_IMAGE_SOURCE, 0);
        cl::copy
        (   queue
        ,   begin (image_source_index)
        ,   end (image_source_index)
        ,   cl_image_source_index
        );

        //  run kernel
        raytrace_kernel
        (   cl::EnqueueArgs (queue, cl::NDRange (RAY_GROUP_SIZE))
        ,   cl_directions
        ,   micpos
        ,   cl_triangles
        ,   ntriangles
        ,   cl_vertices
        ,   source
        ,   cl_surfaces
        ,   cl_impulses
        ,   cl_image_source
        ,   cl_image_source_index
        ,   nreflections
        ,   (VolumeType)
            {{  0.001 * -0.1
            ,   0.001 * -0.2
            ,   0.001 * -0.5
            ,   0.001 * -1.1
            ,   0.001 * -2.7
            ,   0.001 * -9.4
            ,   0.001 * -29.0
            ,   0.001 * -60.0
            }}
        );

        //  copy output to main memory
        cl::copy
        (   queue
        ,   cl_image_source_index
        ,   begin (image_source_index)
        ,   end (image_source_index)
        );
        cl::copy (queue, cl_image_source, begin (image), end (image));

        //  remove duplicate image-source contributions
        for
        (   auto j = 0
        ;   j != RAY_GROUP_SIZE * NUM_IMAGE_SOURCE
        ;   j += NUM_IMAGE_SOURCE
        )
        {
            for (auto k = 1; k != NUM_IMAGE_SOURCE + 1; ++k)
            {
                vector <unsigned long> surfaces
                (   image_source_index.begin() + j
                ,   image_source_index.begin() + j + k
                );

                if (k == 1 || surfaces.back() != 0)
                {
                    auto it = imageSourceTally.find (surfaces);
                    if (it == imageSourceTally.end())
                    {
                        imageSourceTally [surfaces] = image [j + k - 1];
                    }
                }
            }
        }

        cl::copy
        (   queue
        ,   cl_impulses
        ,   storedDiffuse.begin() + b * nreflections
        ,   storedDiffuse.begin() + e * nreflections
        );
    }
}

RaytracerResults Raytracer::getRawDiffuse()
{
    return RaytracerResults (storedDiffuse, storedMicpos);
}

RaytracerResults Raytracer::getRawImages (bool removeDirect)
{
    auto temp = imageSourceTally;
    if (removeDirect)
        temp.erase (vector<unsigned long>{0});

    vector <Impulse> ret (temp.size());
    transform
    (   begin (temp)
    ,   end (temp)
    ,   begin (ret)
    ,   [] (const auto & i) {return i.second;}
    );
    return RaytracerResults (ret, storedMicpos);
}

RaytracerResults Raytracer::getAllRaw (bool removeDirect)
{
    auto diffuse = getRawDiffuse().impulses;
    const auto image = getRawImages (removeDirect).impulses;
    diffuse.insert (diffuse.end(), image.begin(), image.end());
    return RaytracerResults (diffuse, storedMicpos);
}

HrtfAttenuator::HrtfAttenuator()
:   cl_hrtf
    (   cl_context
    ,   CL_MEM_READ_WRITE
    ,   sizeof (VolumeType) * 360 * 180
    )
,   attenuate_kernel
    (   cl::make_kernel
        <   cl_float3
        ,   cl::Buffer
        ,   cl::Buffer
        ,   cl::Buffer
        ,   cl_float3
        ,   cl_float3
        ,   cl_ulong
        > (cl_program, "hrtf")
    )
{

}

vector <vector <AttenuatedImpulse>> HrtfAttenuator::attenuate
(   const RaytracerResults & results
,   const HrtfConfig & config
)
{
    return attenuate (results, config.facing, config.up);
}

vector <vector <AttenuatedImpulse>> HrtfAttenuator::attenuate
(   const RaytracerResults & results
,   const cl_float3 & facing
,   const cl_float3 & up
)
{
    auto channels = {0, 1};
    vector <vector <AttenuatedImpulse>> attenuated (channels.size());
    transform
    (   begin (channels)
    ,   end (channels)
    ,   begin (attenuated)
    ,   [this, &results, facing, up] (auto i) {
            return this->attenuate(results.mic, i, facing, up, results.impulses);
        }
    );
    return attenuated;
}

vector <AttenuatedImpulse> HrtfAttenuator::attenuate
(   const cl_float3 & mic_pos
,   unsigned long channel
,   const cl_float3 & facing
,   const cl_float3 & up
,   const vector <Impulse> & impulses
)
{
    //  muck around with the table format
    vector <VolumeType> hrtfChannelData (360 * 180);
    auto offset = 0;
    for (const auto & i : getHrtfData() [channel])
    {
        copy (begin (i), end (i), hrtfChannelData.begin() + offset);
        offset += i.size();
    }

    //  copy hrtf table to buffer
    cl::copy (queue, begin (hrtfChannelData), end (hrtfChannelData), cl_hrtf);

    //  set up buffers
    cl_in = cl::Buffer
    (   cl_context
    ,   CL_MEM_READ_WRITE
    ,   impulses.size() * sizeof (Impulse)
    );
    cl_out = cl::Buffer
    (   cl_context
    ,   CL_MEM_READ_WRITE
    ,   impulses.size() * sizeof (AttenuatedImpulse)
    );

    //  copy input to buffer
    cl::copy (queue, impulses.begin(), impulses.end(), cl_in);

    //  run kernel
    attenuate_kernel
    (   cl::EnqueueArgs (queue, cl::NDRange (impulses.size()))
    ,   mic_pos
    ,   cl_in
    ,   cl_out
    ,   cl_hrtf
    ,   facing
    ,   up
    ,   channel
    );

    //  create output storage
    vector <AttenuatedImpulse> ret (impulses.size());

    //  copy to output
    cl::copy (queue, cl_out, ret.begin(), ret.end());
    return ret;
}

const array <array <array <cl_float8, 180>, 360>, 2> & HrtfAttenuator::getHrtfData() const
{
    return HRTF_DATA;
}

SpeakerAttenuator::SpeakerAttenuator()
:   attenuate_kernel
    (   cl::make_kernel
        <   cl_float3
        ,   cl::Buffer
        ,   cl::Buffer
        ,   Speaker
        > (cl_program, "attenuate")
    )
{

}

vector <vector <AttenuatedImpulse>> SpeakerAttenuator::attenuate
(   const RaytracerResults & results
,   const vector <Speaker> & speakers
)
{
    vector <vector <AttenuatedImpulse>> attenuated (speakers.size());
    transform
    (   begin (speakers)
    ,   end (speakers)
    ,   begin (attenuated)
    ,   [this, &results] (const auto & i)
        {
            return this->attenuate(results.mic, i, results.impulses);
        }
    );
    return attenuated;
}

vector <AttenuatedImpulse> SpeakerAttenuator::attenuate
(   const cl_float3 & mic_pos
,   const Speaker & speaker
,   const vector <Impulse> & impulses
)
{
    //  init buffers
    cl_in = cl::Buffer
    (   cl_context
    ,   CL_MEM_READ_WRITE
    ,   impulses.size() * sizeof (Impulse)
    );
    cl_out = cl::Buffer
    (   cl_context
    ,   CL_MEM_READ_WRITE
    ,   impulses.size() * sizeof (AttenuatedImpulse)
    );

    //  copy input data to buffer
    cl::copy (queue, impulses.begin(), impulses.end(), cl_in);

    //  run kernel
    attenuate_kernel
    (   cl::EnqueueArgs (queue, cl::NDRange (impulses.size()))
    ,   mic_pos
    ,   cl_in
    ,   cl_out
    ,   speaker
    );

    //  create output location
    vector <AttenuatedImpulse> ret (impulses.size());

    //  copy from buffer to output
    cl::copy (queue, cl_out, ret.begin(), ret.end());
    return ret;
}

void attemptJsonParse (const string & fname, Document & doc)
{
    ifstream in (fname);
    string file
    (   (istreambuf_iterator <char> (in))
    ,   istreambuf_iterator <char>()
    );

    doc.Parse(file.c_str());
}

