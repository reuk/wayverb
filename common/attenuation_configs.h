#pragma once

#include "vec.h"

namespace config {

struct AttenuationModel {
    enum class Mode {
        microphone,
        hrtf,
    };

    virtual Mode get_mode() const = 0;
};

struct Microphone {
    constexpr Microphone(const Vec3f& facing = Vec3f(), float shape = 0)
            : facing(facing)
            , shape(shape) {
    }
    Vec3f facing;
    float shape;
};

struct MicrophoneModel : public AttenuationModel {
    MicrophoneModel(
        const std::vector<Microphone>& microphones = std::vector<Microphone>())
            : microphones(microphones) {
    }

    Mode get_mode() const override {
        return Mode::microphone;
    }

    std::vector<Microphone> microphones;
};

struct HrtfModel : public AttenuationModel {
    Mode get_mode() const override {
        return Mode::hrtf;
    }

    Vec3f facing;
    Vec3f up;
};

}  // namespace config
