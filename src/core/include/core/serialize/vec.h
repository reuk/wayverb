#pragma once

#include "glm/glm.hpp"

#include "cereal/cereal.hpp"

namespace cereal {

template <typename Archive>
void serialize(Archive& archive, glm::vec3& m) {
    size_type s = 3;
    archive(make_size_tag(s));
    if (s != 3) {
        throw std::runtime_error("Vec array length is incorrect.");
    }
    archive(m.x, m.y, m.z);
}

}  // namespace cereal
