#pragma once

#include "core/serialize/vec.h"

#include "core/orientable.h"

template <typename Archive>
void wayverb::core::orientable::serialize(Archive& archive) {
    archive(cereal::make_nvp("pointing", pointing_),
            cereal::make_nvp("up", up_));
}

