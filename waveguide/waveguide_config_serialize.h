#pragma once

#include "app_config_serialize.h"
#include <cereal/types/base_class.hpp>

namespace config {
template <typename Archive>
void serialize(Archive& archive, Waveguide& m) {
    archive(cereal::make_nvp("app", cereal::virtual_base_class<App>(&m)),
            cereal::make_nvp("filter frequency", m.filter_frequency),
            cereal::make_nvp("oversample ratio", m.oversample_ratio));
}
}  // namespace config
JSON_OSTREAM_OVERLOAD(config::Waveguide);
