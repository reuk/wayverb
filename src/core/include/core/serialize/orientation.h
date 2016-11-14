#pragma once

#include "core/serialize/vec.h"

#include "core/orientation.h"

template <typename Archive>
void wayverb::core::orientation::serialize(Archive& archive) {
    archive(cereal::make_nvp("pointing", pointing_),
            cereal::make_nvp("up", up_));
}

