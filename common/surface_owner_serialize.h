#pragma once

#include "json_read_write.h"
#include "surface_owner.h"

#include <cereal/types/vector.hpp>

template <typename Archive>
void serialize(Archive& archive, VolumeType& m) {
    archive(cereal::make_nvp("volumes", m.s));
}
JSON_OSTREAM_OVERLOAD(VolumeType);

template <typename Archive>
void serialize(Archive& archive, Surface& m) {
    archive(cereal::make_nvp("specular", m.specular),
            cereal::make_nvp("diffuse", m.diffuse));
}
JSON_OSTREAM_OVERLOAD(Surface);

template <typename Archive>
void serialize(Archive& archive, SurfaceOwner& m) {
    archive(cereal::make_nvp("surfaces", m.surfaces));
}
JSON_OSTREAM_OVERLOAD(SurfaceOwner);
