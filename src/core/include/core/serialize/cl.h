#pragma once

#include "common/cl_include.h"

namespace core {
namespace cereal {

template <typename Archive>
void serialize(Archive& archive, cl_float3& m) {
    archive(cereal::make_nvp("x", m.s[0]),
            cereal::make_nvp("y", m.s[1]),
            cereal::make_nvp("z", m.s[2]));
}

}  // namespace cereal
}  // namespace core
