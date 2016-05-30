#include "waveguide/hrtf_attenuator.h"

#include "common/filters_common.h"
#include "common/hrtf.h"
#include "common/stl_wrappers.h"

HrtfAttenuator::HrtfAttenuator(const glm::vec3& direction,
                               const glm::vec3& up,
                               int channel,
                               float sr)
        : direction(direction)
        , up(up)
        , channel(channel)
        , sr(sr) {
}

glm::vec3 transform(const glm::vec3& pointing,
                    const glm::vec3& up,
                    const glm::vec3& d) {
    auto x = glm::normalize(glm::cross(up, pointing));
    auto y = glm::cross(pointing, x);
    auto z = pointing;
    return glm::vec3(glm::dot(x, d), glm::dot(y, d), glm::dot(z, d));
}

float azimuth(const glm::vec3& d) {
    return atan2(d.x, d.z);
}

float elevation(const glm::vec3& d) {
    return atan2(d.y, glm::length(glm::vec2(d.x, d.z)));
}

float degrees(float radians) {
    return radians * 180 / M_PI;
}

float HrtfAttenuator::attenuation(const glm::vec3& incident, int band) const {
    auto transformed = transform(direction, up, incident);
    int a = degrees(azimuth(transformed)) + 180;
    a %= 360;
    int e = degrees(elevation(transformed));
    e = 90 - e;
    return HrtfData::HRTF_DATA[channel][a][e].s[band];
}

glm::vec3 HrtfAttenuator::get_direction() const {
    return direction;
}

glm::vec3 HrtfAttenuator::get_up() const {
    return up;
}

std::vector<float> HrtfAttenuator::process(
    const std::vector<RunStepResult>& input) const {
    std::vector<float> ret(input.size(), 0);
    filter::LinkwitzRileyBandpass bandpass;
    for (auto band = 0u; band != sizeof(VolumeType) / sizeof(float); ++band) {
        if (HrtfData::EDGES[band + 1] < sr / 2) {
            std::vector<float> this_band(input.size());
            proc::transform(input, this_band.begin(), [this, band](auto i) {
                //  TODO DEFINITELY CHECK THIS
                //  RUN TESTS YEAH
                auto mag = glm::length(i.intensity);
                if (mag == 0)
                    return 0.0f;
                mag = sqrt(mag * pow(attenuation(i.intensity, band), 2));
                return std::copysign(mag, i.pressure);
            });

            bandpass.set_params(
                HrtfData::EDGES[band], HrtfData::EDGES[band + 1], sr);
            bandpass.filter(this_band);

            proc::transform(
                this_band, ret.begin(), ret.begin(), [](auto a, auto b) {
                    return a + b;
                });
        }
    }

    //  TODO filter with diffuse-field-response filter here
    //  make sure to use zero-phase filtering
    return ret;
}
