#pragma once

#include "raytracer_config.h"
#include "waveguide_config.h"

namespace config {

class Combined : public Waveguide, public Raytracer {
public:
    template<typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::base_class<Waveguide>(this),
                cereal::base_class<Raytracer>(this));
    }
};

}  // namespace config
