#pragma once

#include "scene_data.h"

#include <array>

struct hrtf_data {
    static const std::array<std::array<std::array<volume_type, 180>, 360>, 2>
            data;
    static const std::array<float, 9> edges;
};
