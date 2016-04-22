#pragma once

#include "json_read_write.h"
#include "vec.h"

template <typename Archive, typename T>
void serialize(Archive& archive, Vec3<T>& m) {
    cereal::size_type s = 3;
    archive(cereal::make_size_tag(s));
    if (s != 3) {
        throw std::runtime_error("vec must be of length 3");
    }
    archive(m.x, m.y, m.z);
}
template <typename T>
JSON_OSTREAM_OVERLOAD(Vec3<T>);
