#pragma once

#include "common/scene_data.h"

#include "common/serialize/json_read_write.h"

#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>

template <typename Archive>
void serialize(Archive& archive, VolumeType& m) {
    cereal::size_type s = 8;
    archive(cereal::make_size_tag(s));
    if (s != 8) {
        throw std::runtime_error("volume array must be of length 8");
    }
    proc::for_each(m.s, [&archive](auto& i) { archive(i); });
}
JSON_OSTREAM_OVERLOAD(VolumeType);

template <typename Archive>
void serialize(Archive& archive, Surface& m) {
    archive(cereal::make_nvp("specular", m.specular),
            cereal::make_nvp("diffuse", m.diffuse));
}
JSON_OSTREAM_OVERLOAD(Surface);

template <typename Archive>
void serialize(Archive& archive, SceneData::Material& m) {
    archive(cereal::make_nvp("name", m.name),
            cereal::make_nvp("surface", m.surface));
}
JSON_OSTREAM_OVERLOAD(SceneData::Material);
