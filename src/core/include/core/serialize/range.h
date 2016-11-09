#pragma once

#include "utilities/range.h"

#include "cereal/cereal.hpp"

namespace cereal {

template <typename Archive, typename T>
void load(Archive& archive, util::range<T>& range) {
    T min, max;
    archive(cereal::make_nvp("min", min), cereal::make_nvp("max", max));
    range = util::range<T>{min, max};
}

template <typename Archive, typename T>
void save(Archive& archive, const util::range<T>& range) {
    archive(cereal::make_nvp("min", range.get_min()),
            cereal::make_nvp("max", range.get_max()));
}

}  // namespace cereal

