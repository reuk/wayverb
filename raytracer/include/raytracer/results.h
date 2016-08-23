#pragma once

#include "raytracer/cl/structs.h"

#include "common/aligned/vector.h"

#include "glm/glm.hpp"

#include <experimental/optional>

namespace raytracer {

class results final {
public:
    results(std::experimental::optional<impulse>&& direct,
            aligned::vector<impulse>&& image_source,
            aligned::vector<aligned::vector<impulse>>&& diffuse,
            const glm::vec3& receiver,
            double speed_of_sound);

    aligned::vector<impulse> get_impulses(bool direct = true,
                                          bool image_source = true,
                                          bool diffuse = true) const;

    using impulse_callback = std::function<void(const impulse&)>;
    void for_each_impulse(const impulse_callback& callback,
                          bool direct = true,
                          bool image_source = true,
                          bool diffuse = true) const;

    const std::experimental::optional<impulse>& get_direct() const;
    const aligned::vector<impulse>& get_image_source() const;
    const aligned::vector<aligned::vector<impulse>>& get_diffuse() const;

    glm::vec3 get_receiver() const;
    double get_speed_of_sound() const;

private:
    std::experimental::optional<impulse> direct;
    aligned::vector<impulse> image_source;
    aligned::vector<aligned::vector<impulse>> diffuse;

    glm::vec3 receiver;
    double speed_of_sound;
};

}  // namespace raytracer
