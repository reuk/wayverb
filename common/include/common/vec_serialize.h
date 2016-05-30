#pragma once

#include "json_read_write.h"

#include "glm/glm.hpp"

namespace glm {

template <typename Archive>
void serialize(Archive& archive, vec3& m) {
    cereal::size_type s = 3;
    archive(cereal::make_size_tag(s));
    if (s != 3) {
        throw std::runtime_error("vec must be of length 3");
    }
    archive(m.x, m.y, m.z);
}

}  // namespace glm
JSON_OSTREAM_OVERLOAD(glm::vec3);
