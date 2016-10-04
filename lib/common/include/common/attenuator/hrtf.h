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

volume_type attenuation(const hrtf& hrtf, const glm::vec3& incident);
