#pragma once

#include "combined_config.h"

#include "raytracer/serialize/config.h"
#include "waveguide/serialize/config.h"

namespace config {
template <typename Archive>
void serialize(Archive& archive, Combined& m) {
    archive(cereal::make_nvp("waveguide", cereal::base_class<Waveguide>(&m)),
            cereal::make_nvp("raytracer", cereal::base_class<Raytracer>(&m)));
}
}  // namespace config
JSON_OSTREAM_OVERLOAD(config::Combined);
