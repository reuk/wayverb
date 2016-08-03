#pragma once

#include "common/boundaries.h"
#include "common/serialize/vec.h"

template <typename Archive>
void box::serialize(Archive& archive) {
    archive(cereal::make_nvp("c0", c0), cereal::make_nvp("c1", c1));
}
JSON_OSTREAM_OVERLOAD(box);

template <typename Archive>
void cuboid_boundary::serialize(Archive& archive) {
    archive(cereal::make_nvp("box", boundary));
}
JSON_OSTREAM_OVERLOAD(cuboid_boundary);
