#pragma once

#include "common/cl/scene_structs.h"

#include "glm/glm.hpp"

class hrtf final {
public:
    enum class channel { left, right };

    hrtf(const glm::vec3& pointing, const glm::vec3& up, channel channel);

    glm::vec3 get_pointing() const;
    glm::vec3 get_up() const;
    channel get_channel() const;

    void set_pointing(const glm::vec3& pointing);
    void set_up(const glm::vec3& up);
    void set_channel(channel channel);

private:
    glm::vec3 pointing_;
    glm::vec3 up_;
    channel channel_;
};

glm::vec3 transform(const glm::vec3& pointing,
                    const glm::vec3& up,
                    const glm::vec3& d);

float degrees(float radians);

template <typename It>
constexpr auto to_volume_type(It begin, It end) {
    volume_type ret{};
    for (auto it{std::begin(ret.s)}; begin != end; ++begin, ++it) {
        *it = *begin;
    }
    return ret;
}

volume_type attenuation(const hrtf& hrtf, const glm::vec3& incident);
