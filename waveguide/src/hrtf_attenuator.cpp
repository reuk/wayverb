#include "waveguide/hrtf_attenuator.h"
#include "waveguide/waveguide.h"

#include "common/filters_common.h"
#include "common/hrtf.h"
#include "common/stl_wrappers.h"

namespace {

glm::vec3 transform(const glm::vec3& pointing,
                    const glm::vec3& up,
                    const glm::vec3& d) {
    auto x = glm::normalize(glm::cross(up, pointing));
    auto y = glm::cross(pointing, x);
    auto z = pointing;
    return glm::vec3(glm::dot(x, d), glm::dot(y, d), glm::dot(z, d));
}

float azimuth(const glm::vec3& d) { return atan2(d.x, d.z); }

float elevation(const glm::vec3& d) {
    return atan2(d.y, glm::length(glm::vec2(d.x, d.z)));
}

float degrees(float radians) { return radians * 180 / M_PI; }

float attenuation(const glm::vec3& direction,
                  const glm::vec3& up,
                  HrtfChannel channel,
                  const glm::vec3& incident,
                  int band) {
    auto transformed = transform(direction, up, incident);
    int a            = degrees(azimuth(transformed)) + 180;
    a %= 360;
    int e = degrees(elevation(transformed));
    e     = 90 - e;
    return HrtfData::HRTF_DATA[channel == HrtfChannel::left ? 0 : 1][a][e]
            .s[band];
}

}  // namespace

namespace waveguide {
aligned::vector<aligned::vector<float>> HrtfAttenuator::process(
        const aligned::vector<RunStepResult>& input,
        const glm::vec3& direction,
        const glm::vec3& up,
        HrtfChannel channel) const {
    aligned::vector<aligned::vector<float>> ret;
    for (auto band = 0u; band != HrtfData::EDGES.size(); ++band) {
        aligned::vector<float> this_band;
        this_band.reserve(input.size());
        proc::transform(input,
                        std::back_inserter(this_band),
                        [direction, up, channel, band](auto i) {
                            //  TODO DEFINITELY CHECK THIS
                            //  RUN TESTS YEAH
                            auto mag = glm::length(i.get_intensity());
                            if (mag == 0)
                                return 0.0f;
                            mag = sqrt(mag * pow(attenuation(direction,
                                                             up,
                                                             channel,
                                                             i.get_intensity(),
                                                             band),
                                                 2));
                            return std::copysign(mag, i.get_pressure());
                        });

        ret.push_back(std::move(this_band));
    }

    //  TODO filter with diffuse-field-response filter here
    //  make sure to use zero-phase filtering
    return ret;
}

}  // namespace waveguide
