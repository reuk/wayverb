#include "common/attenuator/hrtf.h"

hrtf::hrtf(const glm::vec3& pointing, const glm::vec3& up, channel channel)
        : pointing_{glm::normalize(pointing)}
        , up_{glm::normalize(up)}
        , channel_{channel} {}

glm::vec3 hrtf::get_pointing() const { return pointing_; }
glm::vec3 hrtf::get_up() const { return up_; }
hrtf::channel hrtf::get_channel() const { return channel_; }

void hrtf::set_pointing(const glm::vec3& pointing) {
    pointing_ = glm::normalize(pointing);
}

void hrtf::set_up(const glm::vec3& up) { up_ = glm::normalize(up); }

void hrtf::set_channel(channel channel) { channel_ = channel; }

//----------------------------------------------------------------------------//

glm::vec3 transform(const glm::vec3& pointing,
                    const glm::vec3& up,
                    const glm::vec3& d) {
    const auto x{glm::normalize(glm::cross(up, pointing))};
    const auto y{glm::cross(pointing, x)};
    const auto z{pointing};
    return glm::vec3(glm::dot(x, d), glm::dot(y, d), glm::dot(z, d));
}

float azimuth(const glm::vec3& d) { return atan2(d.x, d.z); }

float elevation(const glm::vec3& d) {
    return atan2(d.y, glm::length(glm::vec2(d.x, d.z)));
}

float degrees(float radians) { return radians * 180 / M_PI; }

/*
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
*/

volume_type attenuation(const hrtf& hrtf, const glm::vec3& incident) {
    //  TODO - regenerate hrtf data / lookup table
    throw std::runtime_error{"attenuation(hrtf, incident) not implemented"};
}
