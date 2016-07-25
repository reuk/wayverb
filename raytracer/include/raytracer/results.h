#pragma once

#include "raytracer/cl_structs.h"

#include "common/aligned/map.h"
#include "common/aligned/vector.h"
#include "common/cl_include.h"

#include <experimental/optional>

namespace raytracer {

class results final {
public:
    results(const std::experimental::optional<Impulse>& direct,
            const aligned::vector<Impulse>& image_source,
            const aligned::vector<aligned::vector<Impulse>>& diffuse,
            const glm::vec3& receiver);

    aligned::vector<Impulse> get_impulses(bool direct       = true,
                                          bool image_source = true,
                                          bool diffuse      = true) const;

    glm::vec3 get_receiver() const;

private:
    std::experimental::optional<Impulse> direct;
    aligned::vector<Impulse> image_source;
    aligned::vector<aligned::vector<Impulse>> diffuse;

    glm::vec3 receiver;
};

}  // namespace raytracer
