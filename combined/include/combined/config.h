#pragma once

#include "raytracer/config.h"
#include "waveguide/config.h"

namespace config {

class Combined : public Waveguide, public Raytracer {
public:
    template<typename Archive>
    void serialize(Archive& archive);
};

}  // namespace config
