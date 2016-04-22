#pragma once

#include "attenuation_configs.h"
#include "vec.h"

#include <memory>

#define SPEED_OF_SOUND (340.0f)

namespace config {

class App {
public:
    virtual ~App() noexcept = default;

    Vec3f source{0, 0, 0};
    Vec3f mic{0, 0, 0};
    float sample_rate{44100};
    int bit_depth{16};
    std::unique_ptr<AttenuationModel> attenuation_model{
        std::make_unique<MicrophoneModel>(
            MicrophoneModel({Microphone(Vec3f(1, 0, 0), 0)}))};
};
}  // namespace config
