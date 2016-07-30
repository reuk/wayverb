#pragma once
#include "combined/model.h"
#include "common/serialize/vec.h"

#include "cereal/cereal.hpp"
#include "cereal/types/vector.hpp"

template <typename Archive>
void serialize(Archive& archive, AzEl& azel) {
    archive(cereal::make_nvp("azimuth", azel.azimuth),
            cereal::make_nvp("elevation", azel.elevation));
}

namespace model {

template <typename Archive>
void serialize(Archive& archive, Pointer& m) {
    archive(cereal::make_nvp("mode", m.mode),
            cereal::make_nvp("spherical", m.spherical),
            cereal::make_nvp("look_at", m.look_at));
}

//----------------------------------------------------------------------------//

template <typename Archive>
void serialize(Archive& archive, Microphone& m) {
    archive(cereal::make_nvp("pointer", m.pointer),
            cereal::make_nvp("shape", m.shape));
}

//----------------------------------------------------------------------------//

template <typename Archive>
void serialize(Archive& archive, ReceiverSettings& m) {
    archive(cereal::make_nvp("position", m.position),
            cereal::make_nvp("mode", m.mode),
            cereal::make_nvp("microphones", m.microphones),
            cereal::make_nvp("hrtf", m.hrtf));
}

//----------------------------------------------------------------------------//

template <typename Archive>
void serialize(Archive& archive, SingleShot& m) {
    archive(cereal::make_nvp("filter_frequency", m.filter_frequency),
            cereal::make_nvp("oversample_ratio", m.oversample_ratio),
            cereal::make_nvp("rays", m.rays),
            cereal::make_nvp("source", m.source),
            cereal::make_nvp("receiver_settings", m.receiver_settings));
}

//----------------------------------------------------------------------------//

template <typename Archive>
void serialize(Archive& archive, App& m) {
    archive(cereal::make_nvp("filter_frequency", m.filter_frequency),
            cereal::make_nvp("oversample_ratio", m.oversample_ratio),
            cereal::make_nvp("rays", m.rays),
            cereal::make_nvp("source", m.source),
            cereal::make_nvp("receiver_settings", m.receiver_settings));
}

}  // namespace model