#include "raytracer/attenuator_program.h"

#include "raytracer/cl/structs.h"
#include "raytracer/cl/speed_of_sound_declaration.h"

#include "common/cl/geometry.h"
#include "common/cl/geometry_structs.h"
#include "common/cl/scene_structs.h"
#include "common/cl/voxel.h"
#include "common/cl/voxel_structs.h"
#include "common/config.h"

namespace raytracer {

constexpr auto source{R"(
#define NULL (0)

float microphone_attenuation(microphone * speaker, float3 direction);
float microphone_attenuation(microphone * speaker, float3 direction) {
    return ((1 - speaker->coefficient) +
            speaker->coefficient *
                dot(normalize(direction), normalize(speaker->direction)));
}

kernel void microphone_kernel(float3 mic_pos,
                       global impulse * impulsesIn,
                       global attenuated_impulse * impulsesOut,
                       microphone speaker) {
    size_t i = get_global_id(0);
    global impulse * thisImpulse = impulsesIn + i;
    if (any(thisImpulse->volume != 0)) {
        const float ATTENUATION = microphone_attenuation(
            &speaker, get_direction(mic_pos, thisImpulse->position));
        impulsesOut[i] = (attenuated_impulse){thisImpulse->volume * ATTENUATION,
                                             thisImpulse->time};
    }
}

float3 transform(float3 pointing, float3 up, float3 d);
float3 transform(float3 pointing, float3 up, float3 d) {
    float3 x = normalize(cross(up, pointing));
    float3 y = cross(pointing, x);
    float3 z = pointing;

    return (float3)(dot(x, d), dot(y, d), dot(z, d));
}

float azimuth(float3 d);
float azimuth(float3 d) {
    return atan2(d.x, d.z);
}

float elevation(float3 d);
float elevation(float3 d) {
    return atan2(d.y, length(d.xz));
}

volume_type hrtf_attenuation(global volume_type * hrtfData,
                            float3 pointing,
                            float3 up,
                            float3 impulseDirection);
volume_type hrtf_attenuation(global volume_type * hrtfData,
                            float3 pointing,
                            float3 up,
                            float3 impulseDirection) {
    float3 transformed = transform(pointing, up, impulseDirection);

    long a = degrees(azimuth(transformed)) + 180;
    a %= 360;
    long e = degrees(elevation(transformed));
    e = 90 - e;

    return hrtfData[a * 180 + e];
}

kernel void hrtf_kernel(float3 mic_pos,
                 global impulse * impulsesIn,
                 global attenuated_impulse * impulsesOut,
                 global volume_type * hrtfData,
                 float3 pointing,
                 float3 up,
                 ulong channel) {
    size_t i = get_global_id(0);
    const float WIDTH = 0.1;

    float3 ear_pos =
        transform(pointing, up, (float3){channel == 0 ? -WIDTH : WIDTH, 0, 0}) +
        mic_pos;

    global impulse * thisImpulse = impulsesIn + i;

    if (any(thisImpulse->volume != 0)) {
        const volume_type ATTENUATION =
            hrtf_attenuation(hrtfData,
                             pointing,
                             up,
                             get_direction(mic_pos, impulsesIn[i].position));

        const float dist0 = distance(thisImpulse->position, mic_pos);
        const float dist1 = distance(thisImpulse->position, ear_pos);
        const float diff = dist1 - dist0;

        impulsesOut[i] =
            (attenuated_impulse){thisImpulse->volume * ATTENUATION,
                                thisImpulse->time + diff * SECONDS_PER_METER};
    }
}

)"};

attenuator_program::attenuator_program(const cl::Context& context,
                                       const cl::Device& device,
                                       double speed_of_sound)
        : program_wrapper(context,
                          device,
                          std::vector<std::string>{
                                  cl_representation_v<volume_type>,
                                  cl_representation_v<surface>,
                                  cl_representation_v<triangle>,
                                  cl_representation_v<triangle_verts>,
                                  cl_representation_v<reflection>,
                                  cl_representation_v<diffuse_path_info>,
                                  cl_representation_v<impulse>,
                                  cl_representation_v<attenuated_impulse>,
                                  cl_representation_v<microphone>,
                                  cl_representation_v<aabb>,
                                  cl_representation_v<ray>,
                                  cl_representation_v<triangle_inter>,
                                  cl_representation_v<intersection>,
                                  ::cl_sources::geometry,
                                  ::cl_sources::voxel,
                                  ::cl_sources::speed_of_sound_declaration(
                                          speed_of_sound),
                                  source}) {
    assert(speed_of_sound);
}

}  // namespace raytracer
