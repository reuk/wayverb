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

    App(const App& rhs)
            : source(rhs.source)
            , mic(rhs.mic)
            , sample_rate(rhs.sample_rate)
            , bit_depth(rhs.bit_depth)
            , attenuation_model(rhs.attenuation_model
                                    ? rhs.attenuation_model->clone()
                                    : nullptr) {
    }
    App& operator=(const App&) noexcept = default;

    App(App&&) noexcept = default;
    App& operator=(App&&) noexcept = default;

    Vec3f source{0, 0, 0};
    Vec3f mic{0, 0, 0};
    float sample_rate{44100};
    int bit_depth{16};

    std::unique_ptr<AttenuationModel> attenuation_model{
        std::make_unique<MicrophoneModel>(
            MicrophoneModel({Microphone(Vec3f(1, 0, 0), 0)}))};
};
}  // namespace config
