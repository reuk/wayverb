#pragma once
#include "combined/model.h"
#include "common/serialize/vec.h"

#include "cereal/cereal.hpp"
#include "cereal/types/vector.hpp"

namespace cereal {

template <typename Archive>
void serialize(Archive& archive, az_el& azel) {
    archive(cereal::make_nvp("azimuth", azel.azimuth),
            cereal::make_nvp("elevation", azel.elevation));
}

template <typename Archive>
void serialize(Archive& archive, orientable& m) {
    archive(cereal::make_nvp("mode", m.mode),
            cereal::make_nvp("spherical", m.spherical),
            cereal::make_nvp("look_at", m.look_at));
}

////////////////////////////////////////////////////////////////////////////////

template <typename Archive>
void serialize(Archive& archive, microphone& m) {
    archive(cereal::make_nvp("orientable", m.orientable),
            cereal::make_nvp("shape", m.shape));
}

////////////////////////////////////////////////////////////////////////////////

template <typename Archive>
void serialize(Archive& archive, receiver_settings& m) {
    archive(cereal::make_nvp("position", m.position),
            cereal::make_nvp("mode", m.mode),
            cereal::make_nvp("microphones", m.microphones),
            cereal::make_nvp("hrtf", m.hrtf));
}

////////////////////////////////////////////////////////////////////////////////

template <typename Archive>
void serialize(Archive& archive, SingleShot& m) {
    archive(cereal::make_nvp("filter_frequency", m.filter_frequency),
            cereal::make_nvp("oversample_ratio", m.oversample_ratio),
            cereal::make_nvp("rays", m.rays),
            cereal::make_nvp("source", m.source),
            cereal::make_nvp("receiver_settings", m.receiver_settings));
}

////////////////////////////////////////////////////////////////////////////////

template <typename Archive>
void serialize(Archive& archive, App& m) {
    archive(cereal::make_nvp("filter_frequency", m.filter_frequency),
            cereal::make_nvp("oversample_ratio", m.oversample_ratio),
            cereal::make_nvp("rays", m.rays),
            cereal::make_nvp("source", m.source),
            cereal::make_nvp("receiver_settings", m.receiver_settings));
}

}  // namespace cereal
