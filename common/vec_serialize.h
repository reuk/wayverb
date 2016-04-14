#pragma once

#include "json_read_write.h"
#include "vec.h"

template <typename Archive, typename T>
void serialize(Archive& archive, Vec3<T>& m) {
    archive(cereal::make_nvp("x", m.x),
            cereal::make_nvp("y", m.y),
            cereal::make_nvp("z", m.z));
}
template <typename T>
JSON_OSTREAM_OVERLOAD(Vec3<T>);
