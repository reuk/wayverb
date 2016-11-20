#pragma once

#include "core/cl/scene_structs.h"

#include "cereal/types/map.hpp"
#include "cereal/types/vector.hpp"

namespace cereal {

template <typename Archive>
void serialize(Archive& archive, wayverb::core::bands_type& m) {
    cereal::size_type s = wayverb::core::simulation_bands;
    archive(cereal::make_size_tag(s));
    if (s != wayverb::core::simulation_bands) {
        throw std::runtime_error("Volume array length is incorrect.");
    }
    std::for_each(std::begin(m.s), std::end(m.s), [&archive](auto& i) {
        archive(i);
    });
}

template <typename Archive, size_t Bands>
void serialize(Archive& archive, wayverb::core::surface<Bands>& m) {
    archive(cereal::make_nvp("absorption", m.absorption),
            cereal::make_nvp("scattering", m.scattering));
}

}  // namespace cereal
