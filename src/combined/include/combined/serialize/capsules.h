#pragma once

#include "combined/capsules.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/vector.hpp"

template <typename T>
template <typename Archive>
void wayverb::combined::capsule<T>::serialize(Archive& archive) {
    archive(cereal::make_nvp("attenuator", attenuator_));
}

CEREAL_REGISTER_TYPE(
        wayverb::combined::capsule<wayverb::core::attenuator::hrtf>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(
        wayverb::combined::capsule_base,
        model::capsule<wayverb::core::attenuator::hrtf>)

CEREAL_REGISTER_TYPE(
        wayverb::combined::capsule<wayverb::core::attenuator::microphone>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(
        wayverb::combined::capsule_base,
        model::capsule<wayverb::core::attenuator::microphone>)
