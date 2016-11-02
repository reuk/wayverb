#pragma once

#include "waveguide/cl/filter_structs.h"

#include "cereal/cereal.hpp"

namespace cereal {

template <typename Archive>
void serialize(Archive& archive, coefficients_canonical& c) {
    archive(make_nvp("b", c.b), make_nvp("a", c.a));
}

}  // namespace cereal
