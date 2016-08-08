#pragma once

#include "common/boundaries.h"
#include "common/serialize/vec.h"

template <size_t n>
template <typename Archive>
void box<n>::serialize(Archive& archive) {
    archive(cereal::make_nvp("c0", c0), cereal::make_nvp("c1", c1));
}

template<size_t n>
JSON_OSTREAM_OVERLOAD(box<n>);

template <typename Archive>
void cuboid_boundary::serialize(Archive& archive) {
    archive(cereal::make_nvp("box", boundary));
}
JSON_OSTREAM_OVERLOAD(cuboid_boundary);
