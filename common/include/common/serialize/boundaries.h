#pragma once

#include "common/serialize/range.h"

template <typename Archive>
void cuboid_boundary::serialize(Archive& archive) {
    archive(cereal::make_nvp("boundary", boundary));
}
JSON_OSTREAM_OVERLOAD(cuboid_boundary);
