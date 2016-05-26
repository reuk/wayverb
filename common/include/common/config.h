#pragma once

#include "attenuation_configs.h"
#include "vec.h"

#include <memory>

#define SPEED_OF_SOUND (340.0f)

namespace config {

class App {
public:
    App() = default;
    virtual ~App() noexcept = default;

    App(const App& rhs) = default;
    App& operator=(const App& rhs) = default;
    App(App&&) noexcept = default;
    App& operator=(App&&) noexcept = default;

    Vec3f source{0, 0, 0};
    Vec3f mic{0, 0, 0};
    float sample_rate{44100};
    int bit_depth{16};
    ReceiverConfig receiver_config;
};
}  // namespace config
