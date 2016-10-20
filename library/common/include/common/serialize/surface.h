#pragma once

#include "common/scene_data_loader.h"

#include "common/serialize/json_read_write.h"

#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>

template <typename Archive>
void serialize(Archive& archive, bands_type& m) {
    cereal::size_type s = 8;
    archive(cereal::make_size_tag(s));
    if (s != 8) {
        throw std::runtime_error("volume array must be of length 8");
    }
    std::for_each(std::begin(m.s), std::end(m.s), [&archive](auto& i) {
        archive(i);
    });
}
JSON_OSTREAM_OVERLOAD(bands_type);

template <typename Archive, size_t Bands>
void serialize(Archive& archive, surface<Bands>& m) {
    archive(cereal::make_nvp("absorption", m.absorption),
            cereal::make_nvp("scattering", m.scattering));
}
template <size_t Bands>
JSON_OSTREAM_OVERLOAD(surface<Bands>);

template <typename Archive>
void serialize(Archive& archive, scene_data_loader::material& m) {
    archive(cereal::make_nvp("name", m.name),
            cereal::make_nvp("surface", m.surface));
}
JSON_OSTREAM_OVERLOAD(scene_data_loader::material);
