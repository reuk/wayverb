#include "waveguide/attenuator/hrtf.h"
#include "waveguide/waveguide.h"

#include "common/cl/iterator.h"
#include "common/cl/traits.h"
#include "common/filters_common.h"
#include "common/hrtf.h"
#include "common/map_to_vector.h"
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
                  hrtf_channel channel,
                  const glm::vec3& incident,
                  int band) {
    auto transformed = transform(direction, up, incident);
    int a = degrees(azimuth(transformed)) + 180;
    a %= 360;
    int e = degrees(elevation(transformed));
    e = 90 - e;
    return hrtf_data::data[channel == hrtf_channel::left ? 0 : 1][a][e].s[band];
}

}  // namespace

namespace waveguide {
namespace attenuator {

hrtf::hrtf(const glm::vec3& direction,
           const glm::vec3& up,
           hrtf_channel channel)
        : direction_(direction)
        , up_(up)
        , channel_(channel) {}

aligned::vector<volume_type> hrtf::process(
        const aligned::vector<
                waveguide::postprocessor::microphone_state::output>& input)
        const {
    aligned::vector<volume_type> ret(input.size());

    for (auto band{0u}; band != detail::components_v<volume_type>; ++band) {
        proc::transform(
                input, make_cl_type_iterator(ret.begin(), band), [&](auto i) {
                    auto mag{glm::length(i.intensity)};
                    if (mag == 0) {
                        return 0.0f;
                    }
                    mag = sqrt(mag * pow(attenuation(direction_,
                                                     up_,
                                                     channel_,
                                                     i.intensity,
                                                     band),
                                         2));
                    return std::copysign(mag, i.pressure);
                });
    }

    return ret;
}

}  // namespace attenuator
}  // namespace waveguide
