#pragma once

#include "attenuation_configs.h"
#include "vec_serialize.h"

#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>

namespace config {

template <typename Archive>
void serialize(Archive& archive, Microphone& m) {
    archive(cereal::make_nvp("facing", m.facing),
            cereal::make_nvp("shape", m.shape));
}

template <typename Archive>
void serialize(Archive& archive, MicrophoneModel& m) {
    archive(cereal::make_nvp("microphones", m.microphones));
}

template <typename Archive>
void serialize(Archive& archive, HrtfModel& m) {
    archive(cereal::make_nvp("facing", m.facing), cereal::make_nvp("up", m.up));
}

template <typename Archive>
void serialize(Archive& archive, ReceiverConfig& m) {
    archive(cereal::make_nvp("mode", m.mode),
            cereal::make_nvp("microphone model", m.microphone_model),
            cereal::make_nvp("hrtf model", m.hrtf_model));
}

}  // namespace config

CEREAL_REGISTER_TYPE_WITH_NAME(config::MicrophoneModel, "microphone model");
CEREAL_REGISTER_TYPE_WITH_NAME(config::HrtfModel, "hrtf model");

JSON_OSTREAM_OVERLOAD(config::Microphone);
JSON_OSTREAM_OVERLOAD(config::MicrophoneModel);
JSON_OSTREAM_OVERLOAD(config::HrtfModel);
