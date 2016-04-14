#pragma once

#include "vec.h"

#define SPEED_OF_SOUND (340.0f)

namespace config {

class App {
public:
    virtual ~App() noexcept = default;

    Vec3f source{0, 0, 0};
    Vec3f mic{0, 0, 0};
    float sample_rate{44100};
    int bit_depth{16};
};
}  // namespace config
