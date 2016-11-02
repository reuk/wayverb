#pragma once

#include "core/filter_coefficients.h"

#include "cereal/cereal.hpp"
#include "cereal/types/array.hpp"

namespace cereal {

template <typename Archive, size_t B, size_t A>
void serialize(Archive& archive, core::filter_coefficients<B, A>& m) {
    archive(make_nvp("b", m.b), make_nvp("a", m.a));
}

}  // namespace cereal
