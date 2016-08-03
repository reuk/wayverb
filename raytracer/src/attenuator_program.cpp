#include "raytracer/attenuator_program.h"

#include "cl/geometry.h"
#include "cl/structs.h"
#include "cl/voxel.h"

namespace raytracer {

attenuator_program::attenuator_program(const cl::Context& context,
                                       const cl::Device& device)
        : program_wrapper(context,
                          device,
                          std::vector<std::string>{cl_sources::structs,
                                                   cl_sources::geometry,
                                                   cl_sources::voxel,
                                                   source}) {}

static_assert(SPEED_OF_SOUND != 0, "SPEED_OF_SOUND");

const std::string attenuator_program::source("#define SPEED_OF_SOUND " +
                                             std::to_string(SPEED_OF_SOUND) +
                                             "\n"
                                             R"(

#define NULL (0)

constant float SECONDS_PER_METER = 1.0f / SPEED_OF_SOUND;

float microphone_attenuation(Microphone * speaker, float3 direction);
float microphone_attenuation(Microphone * speaker, float3 direction) {
    return ((1 - speaker->coefficient) +
            speaker->coefficient *
                dot(normalize(direction), normalize(speaker->direction)));
}

kernel void microphone(float3 mic_pos,
                       global Impulse * impulsesIn,
                       global AttenuatedImpulse * impulsesOut,
                       Microphone speaker) {
    size_t i = get_global_id(0);
    global Impulse * thisImpulse = impulsesIn + i;
    if (any(thisImpulse->volume != 0)) {
        const float ATTENUATION = microphone_attenuation(
            &speaker, get_direction(mic_pos, thisImpulse->position));
        impulsesOut[i] = (AttenuatedImpulse){thisImpulse->volume * ATTENUATION,
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

VolumeType hrtf_attenuation(global VolumeType * hrtfData,
                            float3 pointing,
                            float3 up,
                            float3 impulseDirection);
VolumeType hrtf_attenuation(global VolumeType * hrtfData,
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

kernel void hrtf(float3 mic_pos,
                 global Impulse * impulsesIn,
                 global AttenuatedImpulse * impulsesOut,
                 global VolumeType * hrtfData,
                 float3 pointing,
                 float3 up,
                 ulong channel) {
    size_t i = get_global_id(0);
    const float WIDTH = 0.1;

    float3 ear_pos =
        transform(pointing, up, (float3){channel == 0 ? -WIDTH : WIDTH, 0, 0}) +
        mic_pos;

    global Impulse * thisImpulse = impulsesIn + i;

    if (any(thisImpulse->volume != 0)) {
        const VolumeType ATTENUATION =
            hrtf_attenuation(hrtfData,
                             pointing,
                             up,
                             get_direction(mic_pos, impulsesIn[i].position));

        const float dist0 = distance(thisImpulse->position, mic_pos);
        const float dist1 = distance(thisImpulse->position, ear_pos);
        const float diff = dist1 - dist0;

        impulsesOut[i] =
            (AttenuatedImpulse){thisImpulse->volume * ATTENUATION,
                                thisImpulse->time + diff * SECONDS_PER_METER};
    }
}

)");

}  // namespace raytracer
