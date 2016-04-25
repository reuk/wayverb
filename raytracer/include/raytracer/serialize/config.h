#pragma once

#include "raytracer/config.h"

#include "common/app_config_serialize.h"

#include <cereal/types/base_class.hpp>

namespace config {
template <typename Archive>
void serialize(Archive& archive, Raytracer& m) {
    archive(cereal::make_nvp("app", cereal::virtual_base_class<App>(&m)),
            cereal::make_nvp("rays", m.rays),
            cereal::make_nvp("impulses", m.impulses),
            cereal::make_nvp("hipass", m.ray_hipass),
            cereal::make_nvp("normalize", m.do_normalize),
            cereal::make_nvp("pre trim", m.trim_predelay),
            cereal::make_nvp("post trim", m.trim_tail),
            cereal::make_nvp("remove direct", m.remove_direct),
            cereal::make_nvp("volume scale", m.volume_scale));
}
}  // namespace config
JSON_OSTREAM_OVERLOAD(config::Raytracer);
