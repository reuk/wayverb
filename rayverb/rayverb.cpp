#include "rayverb.h"
#include "filters.h"
#include "config.h"
#include "test_flag.h"
#include "hrtf.h"
#include "boundaries.h"
#include "conversions.h"
#include "geometric.h"
#include "voxel_collection.h"

#include "logger.h"

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
#include <random>

static auto volume_factor = 0.001f;
static const VolumeType AIR_COEFFICIENT{{
    volume_factor * -0.1f,
    volume_factor * -0.2f,
    volume_factor * -0.5f,
    volume_factor * -1.1f,
    volume_factor * -2.7f,
    volume_factor * -9.4f,
    volume_factor * -29.0f,
    volume_factor * -60.0f,
}};

std::vector<std::vector<std::vector<float>>> flattenImpulses(
    const std::vector<std::vector<AttenuatedImpulse>>& attenuated,
    float samplerate) {
    std::vector<std::vector<std::vector<float>>> flattened(attenuated.size());
    transform(
        begin(attenuated),
        end(attenuated),
        begin(flattened),
        [samplerate](const auto& i) { return flattenImpulses(i, samplerate); });
    return flattened;
}

/// Turn a collection of AttenuatedImpulses into a vector of 8 vectors, where
/// each of the 8 vectors represent sample values in a different frequency band.
std::vector<std::vector<float>> flattenImpulses(
    const std::vector<AttenuatedImpulse>& impulse, float samplerate) {
    const auto MAX_TIME_LIMIT = 20.0f;
    // Find the index of the final sample based on time and samplerate
    float maxtime = 0;
    for (const auto& i : impulse)
        maxtime = std::max(maxtime, i.time);
    maxtime = std::min(maxtime, MAX_TIME_LIMIT);
    const auto MAX_SAMPLE = round(maxtime * samplerate) + 1;

    //  Create somewhere to store the results.
    std::vector<std::vector<float>> flattened(
        sizeof(VolumeType) / sizeof(float), std::vector<float>(MAX_SAMPLE, 0));

    //  For each impulse, calculate its index, then add the impulse's volumes
    //  to the volumes already in the output array.
    for (const auto& i : impulse) {
        const auto SAMPLE = round(i.time * samplerate);
        if (SAMPLE < MAX_SAMPLE) {
            for (auto j = 0u; j != flattened.size(); ++j) {
                flattened[j][SAMPLE] += i.volume.s[j];
            }
        }
    }

    return flattened;
}

/// Sum a collection of vectors of the same length into a single vector
std::vector<float> mixdown(const std::vector<std::vector<float>>& data) {
    std::vector<float> ret(data.front().size(), 0);
    for (auto&& i : data)
        transform(
            ret.begin(), ret.end(), i.begin(), ret.begin(), std::plus<float>());
    return ret;
}

/// Find the index of the last sample with an amplitude of minVol or higher,
/// then resize the vectors down to this length.
void trimTail(std::vector<std::vector<float>>& audioChannels, float minVol) {
    using index_type =
        std::common_type_t<std::iterator_traits<std::vector<
                               float>::reverse_iterator>::difference_type,
                           int>;

    // Find last index of required amplitude or greater.
    auto len = std::accumulate(
        audioChannels.begin(),
        audioChannels.end(),
        0,
        [minVol](auto current, const auto& i) {
            return std::max(
                index_type{current},
                index_type{distance(i.begin(),
                                    std::find_if(i.rbegin(),
                                                 i.rend(),
                                                 [minVol](auto j) {
                                                     return std::abs(j) >=
                                                            minVol;
                                                 })
                                        .base()) -
                           1});
        });

    // Resize.
    for (auto&& i : audioChannels)
        i.resize(len);
}

/// Collects together all the post-processing steps.
std::vector<std::vector<float>> process(
    FilterType filtertype,
    std::vector<std::vector<std::vector<float>>>& data,
    float sr,
    bool do_normalize,
    float lo_cutoff,
    bool do_trim_tail,
    float volume_scale) {
    filter(filtertype, data, sr, lo_cutoff);
    std::vector<std::vector<float>> ret(data.size());
    transform(data.begin(), data.end(), ret.begin(), mixdown);

    if (do_normalize)
        normalize(ret);

    if (volume_scale != 1)
        mul(ret, volume_scale);

    if (do_trim_tail)
        trimTail(ret, 0.00001);

    return ret;
}

/// Call binary operation u on pairs of elements from a and b, where a and b are
/// cl_floatx types.
template <typename T, typename U>
inline T elementwise(const T& a, const T& b, const U& u) {
    T ret;
    std::transform(
        std::begin(a.s), std::end(a.s), std::begin(b.s), std::begin(ret.s), u);
    return ret;
}

cl_float3 sphere_point(float z, float theta) {
    const float ztemp = sqrtf(1 - z * z);
    return (cl_float3){{ztemp * cosf(theta), ztemp * sinf(theta), z, 0}};
}

std::vector<cl_float3> get_random_directions(unsigned long num) {
    std::vector<cl_float3> ret(num);
    std::uniform_real_distribution<float> z(-1, 1);
    std::uniform_real_distribution<float> theta(-M_PI, M_PI);
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine engine(seed);

    for (auto& i : ret)
        i = sphere_point(z(engine), theta(engine));

    return ret;
}

RaytracerResults Results::get_diffuse() const {
    return RaytracerResults(diffuse, mic, source, rays, reflections);
}

RaytracerResults Results::get_image_source(bool remove_direct) const {
    auto temp = image_source;
    if (remove_direct)
        temp.erase(std::vector<unsigned long>{0});

    std::vector<Impulse> ret(temp.size());
    transform(begin(temp),
              end(temp),
              begin(ret),
              [](const auto& i) { return i.second; });
    return RaytracerResults(ret, mic, source, rays, reflections);
}

RaytracerResults Results::get_all(bool remove_direct) const {
    auto diffuse = get_diffuse().impulses;
    const auto image = get_image_source(remove_direct).impulses;
    diffuse.insert(diffuse.end(), image.begin(), image.end());
    return RaytracerResults(diffuse, mic, source, rays, reflections);
}

//----------------------------------------------------------------------------//

BaseRaytrace::BaseRaytrace(const RayverbProgram& program,
                           cl::CommandQueue& queue)
        : queue(queue)
        , context(program.getInfo<CL_PROGRAM_CONTEXT>()) {
}

Results BaseRaytrace::run(const SceneData& scene_data,
                          const Vec3f& micpos,
                          const Vec3f& source,
                          int rays,
                          int reflections) {
    return run(
        scene_data, micpos, source, get_random_directions(rays), reflections);
}

const cl::Context& BaseRaytrace::get_context() const {
    return context;
}

cl::CommandQueue& BaseRaytrace::get_queue() {
    return queue;
}

//----------------------------------------------------------------------------//

void remove_duplicates(const std::vector<cl_ulong>& path,
                       const std::vector<Impulse>& image,
                       const int RAY_GROUP_SIZE,
                       Results& results) {
    for (auto j = 0; j != RAY_GROUP_SIZE * NUM_IMAGE_SOURCE;
         j += NUM_IMAGE_SOURCE) {
        for (auto k = 1; k != NUM_IMAGE_SOURCE + 1; ++k) {
            std::vector<unsigned long> surfaces(path.begin() + j,
                                                path.begin() + j + k);

            if (k == 1 || surfaces.back() != 0) {
                auto it = results.image_source.find(surfaces);
                if (it == results.image_source.end()) {
                    results.image_source[surfaces] = image[j + k - 1];
                }
            }
        }
    }
}

Raytrace::Raytrace(const RayverbProgram& program, cl::CommandQueue& queue)
        : BaseRaytrace(program, queue)
        , kernel(program.get_raytrace_kernel()) {
}

VolumeType attenuation_for_distance(float distance) {
    VolumeType ret;
    std::transform(std::begin(AIR_COEFFICIENT.s),
                   std::end(AIR_COEFFICIENT.s),
                   std::begin(ret.s),
                   [distance](auto i) { return pow(M_E, distance * i); });
    return ret;
}

void add_direct_impulse(const Vec3f& micpos,
                        const Vec3f& source,
                        const SceneData& scene_data,
                        Results& results) {
    if (geo::point_intersection(micpos,
                                source,
                                scene_data.get_triangles(),
                                scene_data.get_converted_vertices())) {
        auto init_diff = source - micpos;
        auto init_dist = init_diff.mag();
        results.image_source[{0}] = Impulse{
            attenuation_for_distance(init_dist),
            to_cl_float3(micpos + init_diff),
            init_dist / SPEED_OF_SOUND,
        };
    }
}

Results Raytrace::run(const SceneData& scene_data,
                      const Vec3f& micpos,
                      const Vec3f& source,
                      const std::vector<cl_float3>& directions,
                      int reflections) {
    auto RAY_GROUP_SIZE = 4096u;

    //  init buffers
    cl::Buffer cl_directions(
        get_context(), CL_MEM_READ_WRITE, RAY_GROUP_SIZE * sizeof(cl_float3));
    cl::Buffer cl_triangles(
        get_context(),
        begin(const_cast<std::vector<Triangle>&>(scene_data.get_triangles())),
        end(const_cast<std::vector<Triangle>&>(scene_data.get_triangles())),
        false);
    cl::Buffer cl_vertices(
        get_context(),
        begin(const_cast<std::vector<cl_float3>&>(scene_data.get_vertices())),
        end(const_cast<std::vector<cl_float3>&>(scene_data.get_vertices())),
        false);
    cl::Buffer cl_surfaces(
        get_context(),
        begin(const_cast<std::vector<Surface>&>(scene_data.get_surfaces())),
        end(const_cast<std::vector<Surface>&>(scene_data.get_surfaces())),
        false);
    cl::Buffer cl_impulses(get_context(),
                           CL_MEM_READ_WRITE,
                           RAY_GROUP_SIZE * reflections * sizeof(Impulse));
    cl::Buffer cl_image_source(
        get_context(),
        CL_MEM_READ_WRITE,
        RAY_GROUP_SIZE * NUM_IMAGE_SOURCE * sizeof(Impulse));
    cl::Buffer cl_image_source_index(
        get_context(),
        CL_MEM_READ_WRITE,
        RAY_GROUP_SIZE * NUM_IMAGE_SOURCE * sizeof(cl_ulong));

    Results results;
    results.mic = micpos;
    results.source = source;
    results.rays = directions.size();
    results.reflections = reflections;
    results.diffuse.resize(directions.size() * reflections);

    add_direct_impulse(micpos, source, scene_data, results);

    for (auto i = 0u; i != ceil(directions.size() / float(RAY_GROUP_SIZE));
         ++i) {
        using index_type = std::common_type_t<decltype(i* RAY_GROUP_SIZE),
                                              decltype(directions.size())>;
        auto b = i * RAY_GROUP_SIZE;
        auto e = std::min(index_type{directions.size()},
                          index_type{(i + 1) * RAY_GROUP_SIZE});

        //  copy input to buffer
        cl::copy(get_queue(),
                 directions.begin() + b,
                 directions.begin() + e,
                 cl_directions);

        //  zero out impulse storage memory
        std::vector<Impulse> diffuse(
            RAY_GROUP_SIZE * reflections,
            Impulse{{{0, 0, 0, 0, 0, 0, 0, 0}}, {{0, 0, 0}}, 0});
        cl::copy(get_queue(), begin(diffuse), end(diffuse), cl_impulses);

        std::vector<Impulse> image(
            RAY_GROUP_SIZE * NUM_IMAGE_SOURCE,
            Impulse{{{0, 0, 0, 0, 0, 0, 0, 0}}, {{0, 0, 0}}, 0});
        cl::copy(get_queue(), begin(image), end(image), cl_image_source);

        std::vector<cl_ulong> image_source_index(
            RAY_GROUP_SIZE * NUM_IMAGE_SOURCE, 0);
        cl::copy(get_queue(),
                 begin(image_source_index),
                 end(image_source_index),
                 cl_image_source_index);

        //  run kernel
        kernel(cl::EnqueueArgs(get_queue(), cl::NDRange(RAY_GROUP_SIZE)),
               cl_directions,
               to_cl_float3(micpos),
               cl_triangles,
               scene_data.get_triangles().size(),
               cl_vertices,
               to_cl_float3(source),
               cl_surfaces,
               cl_impulses,
               cl_image_source,
               cl_image_source_index,
               reflections,
               AIR_COEFFICIENT);

        //  copy output to main memory
        cl::copy(get_queue(),
                 cl_image_source_index,
                 begin(image_source_index),
                 end(image_source_index));
        cl::copy(get_queue(), cl_image_source, begin(image), end(image));

        remove_duplicates(image_source_index, image, RAY_GROUP_SIZE, results);

        cl::copy(get_queue(),
                 cl_impulses,
                 results.diffuse.begin() + b * reflections,
                 results.diffuse.begin() + e * reflections);
    }

    return results;
}

//----------------------------------------------------------------------------//

ImprovedRaytrace::ImprovedRaytrace(const RayverbProgram& program,
                                   cl::CommandQueue& queue)
        : BaseRaytrace(program, queue)
        , kernel(program.get_improved_raytrace_kernel()) {
}

template <typename T>
auto transpose(const T& t, int x, int y) {
    assert(t.size() == x * y);

    auto c = t;
    for (auto i = 0u; i != x; ++i) {
        for (auto j = 0u; j != y; ++j) {
            c[j + i * y] = t[i + j * x];
        }
    }
    return c;
}

Results ImprovedRaytrace::run(const SceneData& scene_data,
                              const Vec3f& micpos,
                              const Vec3f& source,
                              const std::vector<cl_float3>& directions,
                              int reflections) {
    VoxelCollection vox(scene_data, 4, 0.1);
    auto flattened_vox = vox.get_flattened();

    auto rays = directions.size();
    cl::Buffer cl_ray_info(
        get_context(), CL_MEM_READ_WRITE, rays * sizeof(RayInfo));

    std::vector<RayInfo> ray_info(directions.size());
    std::transform(directions.begin(),
                   directions.end(),
                   ray_info.begin(),
                   [micpos, source](const auto& i) {
                       RayInfo ret;
                       ret.ray.direction = i;
                       ret.ray.position = to_cl_float3(source);
                       ret.distance = 0;
                       ret.volume = VolumeType{{1, 1, 1, 1, 1, 1, 1, 1}};
                       ret.cont = 1;

                       ret.mic_reflection = to_cl_float3(micpos);
                       return ret;
                   });

    cl::copy(get_queue(), begin(ray_info), end(ray_info), cl_ray_info);

    cl::Buffer cl_triangles(
        get_context(),
        begin(const_cast<std::vector<Triangle>&>(scene_data.get_triangles())),
        end(const_cast<std::vector<Triangle>&>(scene_data.get_triangles())),
        false);
    cl::Buffer cl_vertices(
        get_context(),
        begin(const_cast<std::vector<cl_float3>&>(scene_data.get_vertices())),
        end(const_cast<std::vector<cl_float3>&>(scene_data.get_vertices())),
        false);
    cl::Buffer cl_surfaces(
        get_context(),
        begin(const_cast<std::vector<Surface>&>(scene_data.get_surfaces())),
        end(const_cast<std::vector<Surface>&>(scene_data.get_surfaces())),
        false);
    cl::Buffer cl_impulses(
        get_context(), CL_MEM_READ_WRITE, rays * sizeof(Impulse));
    cl::Buffer cl_image_source(
        get_context(), CL_MEM_READ_WRITE, rays * sizeof(Impulse));
    cl::Buffer cl_image_source_index(
        get_context(), CL_MEM_READ_WRITE, rays * sizeof(cl_ulong));

    cl::Buffer cl_voxel_index(
        get_context(), begin(flattened_vox), end(flattened_vox), false);

    Results results;
    results.mic = micpos;
    results.source = source;
    results.rays = directions.size();
    results.reflections = reflections;
    results.diffuse.resize(rays * reflections);

    add_direct_impulse(micpos, source, scene_data, results);

    std::vector<cl_ulong> image_source_index(rays * NUM_IMAGE_SOURCE, 0);
    std::vector<Impulse> image_source(
        rays * NUM_IMAGE_SOURCE,
        Impulse{{{0, 0, 0, 0, 0, 0, 0, 0}}, {{0, 0, 0}}, 0});

    AABB global_aabb{to_cl_float3(vox.get_aabb().get_c0()),
                     to_cl_float3(vox.get_aabb().get_c1())};

    for (auto i = 0u; i != reflections; ++i) {
        auto b = (i + 0) * rays;
        auto e = (i + 1) * rays;

        if (i < NUM_IMAGE_SOURCE) {
            cl::copy(get_queue(),
                     image_source_index.begin() + b,
                     image_source_index.begin() + e,
                     cl_image_source_index);
            cl::copy(get_queue(),
                     image_source.begin() + b,
                     image_source.begin() + e,
                     cl_image_source);
        }

        kernel(cl::EnqueueArgs(get_queue(), cl::NDRange(rays)),
               cl_ray_info,
               cl_voxel_index,
               global_aabb,
               vox.get_side(),
               cl_triangles,
               scene_data.get_triangles().size(),
               cl_vertices,
               cl_surfaces,
               to_cl_float3(source),
               to_cl_float3(micpos),
               cl_impulses,
               cl_image_source,
               cl_image_source_index,
               AIR_COEFFICIENT,
               i);

        cl::copy(get_queue(),
                 cl_impulses,
                 results.diffuse.begin() + b,
                 results.diffuse.begin() + e);

        if (i < NUM_IMAGE_SOURCE) {
            cl::copy(get_queue(),
                     cl_image_source_index,
                     image_source_index.begin() + b,
                     image_source_index.begin() + e);
            cl::copy(get_queue(),
                     cl_image_source,
                     image_source.begin() + b,
                     image_source.begin() + e);
        }
    }

    results.diffuse = transpose(results.diffuse, rays, reflections);

    image_source = transpose(image_source, rays, NUM_IMAGE_SOURCE);
    image_source_index = transpose(image_source_index, rays, NUM_IMAGE_SOURCE);

    remove_duplicates(image_source_index, image_source, rays, results);

    return results;
}

//----------------------------------------------------------------------------//

Hrtf::Hrtf(const RayverbProgram& program, cl::CommandQueue& queue)
        : queue(queue)
        , kernel(program.get_hrtf_kernel())
        , context(program.getInfo<CL_PROGRAM_CONTEXT>())
        , cl_hrtf(context, CL_MEM_READ_WRITE, sizeof(VolumeType) * 360 * 180) {
}

std::vector<std::vector<AttenuatedImpulse>> Hrtf::attenuate(
    const RaytracerResults& results, const Config& config) {
    return attenuate(results, config.facing, config.up);
}

std::vector<std::vector<AttenuatedImpulse>> Hrtf::attenuate(
    const RaytracerResults& results, const Vec3f& facing, const Vec3f& up) {
    auto channels = {0, 1};
    std::vector<std::vector<AttenuatedImpulse>> attenuated(channels.size());
    transform(begin(channels),
              end(channels),
              begin(attenuated),
              [this, &results, facing, up](auto i) {
                  return this->attenuate(to_cl_float3(results.mic),
                                         i,
                                         to_cl_float3(facing),
                                         to_cl_float3(up),
                                         results.impulses);
              });
    return attenuated;
}

std::vector<AttenuatedImpulse> Hrtf::attenuate(
    const cl_float3& mic_pos,
    unsigned long channel,
    const cl_float3& facing,
    const cl_float3& up,
    const std::vector<Impulse>& impulses) {
    //  muck around with the table format
    std::vector<VolumeType> hrtfChannelData(360 * 180);
    auto offset = 0;
    for (const auto& i : getHrtfData()[channel]) {
        copy(begin(i), end(i), hrtfChannelData.begin() + offset);
        offset += i.size();
    }

    //  copy hrtf table to buffer
    cl::copy(queue, begin(hrtfChannelData), end(hrtfChannelData), cl_hrtf);

    //  set up buffers
    cl_in = cl::Buffer(
        context, CL_MEM_READ_WRITE, impulses.size() * sizeof(Impulse));
    cl_out = cl::Buffer(context,
                        CL_MEM_READ_WRITE,
                        impulses.size() * sizeof(AttenuatedImpulse));

    //  copy input to buffer
    cl::copy(queue, impulses.begin(), impulses.end(), cl_in);

    //  run kernel
    kernel(cl::EnqueueArgs(queue, cl::NDRange(impulses.size())),
           mic_pos,
           cl_in,
           cl_out,
           cl_hrtf,
           facing,
           up,
           channel);

    //  create output storage
    std::vector<AttenuatedImpulse> ret(impulses.size());

    //  copy to output
    cl::copy(queue, cl_out, ret.begin(), ret.end());

    return ret;
}

const std::array<std::array<std::array<cl_float8, 180>, 360>, 2>&
Hrtf::getHrtfData() const {
    return HrtfData::HRTF_DATA;
}

Attenuate::Attenuate(const RayverbProgram& program, cl::CommandQueue& queue)
        : queue(queue)
        , kernel(program.get_attenuate_kernel())
        , context(program.getInfo<CL_PROGRAM_CONTEXT>()) {
}

std::vector<std::vector<AttenuatedImpulse>> Attenuate::attenuate(
    const RaytracerResults& results, const std::vector<Speaker>& speakers) {
    std::vector<std::vector<AttenuatedImpulse>> attenuated(speakers.size());
    transform(begin(speakers),
              end(speakers),
              begin(attenuated),
              [this, &results](const auto& i) {
                  return this->attenuate(results.mic, i, results.impulses);
              });
    return attenuated;
}

std::vector<AttenuatedImpulse> Attenuate::attenuate(
    const Vec3f& mic_pos,
    const Speaker& speaker,
    const std::vector<Impulse>& impulses) {
    //  init buffers
    cl_in = cl::Buffer(
        context, CL_MEM_READ_WRITE, impulses.size() * sizeof(Impulse));

    cl_out = cl::Buffer(context,
                        CL_MEM_READ_WRITE,
                        impulses.size() * sizeof(AttenuatedImpulse));
    std::vector<AttenuatedImpulse> zero(
        impulses.size(), AttenuatedImpulse{{{0, 0, 0, 0, 0, 0, 0, 0}}, 0});
    cl::copy(queue, zero.begin(), zero.end(), cl_out);

    //  copy input data to buffer
    cl::copy(queue, impulses.begin(), impulses.end(), cl_in);

    //  run kernel
    kernel(cl::EnqueueArgs(queue, cl::NDRange(impulses.size())),
           to_cl_float3(mic_pos),
           cl_in,
           cl_out,
           speaker);

    //  create output location
    std::vector<AttenuatedImpulse> ret(impulses.size());

    //  copy from buffer to output
    cl::copy(queue, cl_out, ret.begin(), ret.end());

    return ret;
}
