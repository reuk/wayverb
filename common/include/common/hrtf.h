#pragma once

#include "scene_data.h"

#include <array>

namespace hrtf_data {
extern const std::array<std::array<std::array<volume_type, 180>, 360>, 2> data;
extern const std::array<float, 9> edges;
}  // namespace hrtf_data
