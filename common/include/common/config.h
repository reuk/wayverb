#pragma once

#include "attenuation_configs.h"

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

    glm::vec3 source{0, 0, 0};
    glm::vec3 mic{0, 0, 0};
    float sample_rate{44100};
    int bit_depth{16};
    ReceiverConfig receiver_config;
};
}  // namespace config
