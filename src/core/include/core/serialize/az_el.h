#pragma once

#include "core/az_el.h"

#include "cereal/cereal.hpp"

namespace cereal {

template <typename Archive>
void serialize(Archive& archive, wayverb::core::az_el& az_el) {
    archive(cereal::make_nvp("azimuth", az_el.azimuth),
            cereal::make_nvp("elevation", az_el.elevation));
}

}  // namespace cereal
