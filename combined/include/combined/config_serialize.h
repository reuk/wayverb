#pragma once

#include "config.h"

#include "raytracer/serialize/config.h"
#include "waveguide/serialize/config.h"

namespace config {
template <typename Archive>
void Combined::serialize(Archive& archive) {
    archive(cereal::make_nvp("waveguide", cereal::base_class<Waveguide>(this)),
            cereal::make_nvp("raytracer", cereal::base_class<Raytracer>(this)));
}
}  // namespace config
JSON_OSTREAM_OVERLOAD(config::Combined);
