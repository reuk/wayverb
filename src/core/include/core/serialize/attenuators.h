#pragma once

#include "core/serialize/orientation.h"

#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"
#include "core/attenuator/null.h"

template <typename Archive>
void wayverb::core::attenuator::hrtf::serialize(Archive& archive) {
    archive(cereal::make_nvp("orientation", orientation),
            cereal::make_nvp("channel", channel_),
            cereal::make_nvp("radius", radius_));
}

template <typename Archive>
void wayverb::core::attenuator::microphone::serialize(Archive& archive) {
    archive(cereal::make_nvp("orientation", orientation),
            cereal::make_nvp("shape", shape_));
}
