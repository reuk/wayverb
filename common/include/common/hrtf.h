#pragma once

#include "scene_data.h"

#include <array>

struct HrtfData {
    static const std::array<std::array<std::array<VolumeType, 180>, 360>, 2>
            HRTF_DATA;
    static const std::array<float, 9> EDGES;
};
