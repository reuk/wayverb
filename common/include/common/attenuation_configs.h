#pragma once

#include "glm/glm.hpp"

#include <memory>
#include <vector>

namespace config {

struct AttenuationModel {
    enum class Mode {
        microphone,
        hrtf,
    };

    AttenuationModel() = default;
    virtual ~AttenuationModel() noexcept = default;

    AttenuationModel(const AttenuationModel&) = default;
    AttenuationModel& operator=(const AttenuationModel&) noexcept = default;
    AttenuationModel(AttenuationModel&&) noexcept = default;
    AttenuationModel& operator=(AttenuationModel&&) noexcept = default;

    virtual Mode get_mode() const = 0;
    virtual std::unique_ptr<AttenuationModel> clone() const = 0;
};

struct Microphone {
    glm::vec3 facing{0, 0, 1};
    float shape{0};
};

struct MicrophoneModel : public AttenuationModel {
    std::unique_ptr<AttenuationModel> clone() const override {
        return std::unique_ptr<AttenuationModel>(
                std::make_unique<MicrophoneModel>(*this));
    }

    Mode get_mode() const override {
        return Mode::microphone;
    }

    std::vector<Microphone> microphones{Microphone{}};
};

struct HrtfModel : public AttenuationModel {
    Mode get_mode() const override {
        return Mode::hrtf;
    }

    std::unique_ptr<AttenuationModel> clone() const override {
        return std::unique_ptr<AttenuationModel>(
                std::make_unique<HrtfModel>(*this));
    }

    glm::vec3 facing{0, 0, 1};
    glm::vec3 up{0, 1, 0};
};

struct ReceiverConfig {
    config::AttenuationModel::Mode mode;
    config::MicrophoneModel microphone_model;
    config::HrtfModel hrtf_model;
};

}  // namespace config
