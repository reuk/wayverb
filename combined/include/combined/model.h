#pragma once

#include "common/orientable.h"
#include "common/config.h"
#include "common/vec_serialize.h"

#include "cereal/cereal.hpp"

#include <vector>

template <typename Archive>
void serialize(Archive& archive, Orientable::AzEl& azel) {
    archive(cereal::make_nvp("azimuth", azel.azimuth),
            cereal::make_nvp("elevation", azel.elevation));
}

namespace model {

struct Pointer {
    enum class Mode { spherical, look_at };

    inline glm::vec3 get_pointing(const glm::vec3& position) const {
        switch (mode) {
            case Mode::spherical:
                return Orientable::compute_pointing(spherical);
            case Mode::look_at:
                return glm::normalize(look_at - position);
        }
    }

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("mode", mode),
                cereal::make_nvp("spherical", spherical),
                cereal::make_nvp("look_at", look_at));
    }

    Mode mode{Mode::spherical};
    Orientable::AzEl spherical{};
    glm::vec3 look_at{0};
};

struct Microphone {
    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("pointer", pointer),
                cereal::make_nvp("shape", shape));
    }

    Pointer pointer{};
    float shape{0};
};

struct ReceiverSettings {
    enum class Mode { microphones, hrtf };

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("mode", mode),
                cereal::make_nvp("microphones", microphones),
                cereal::make_nvp("hrtf", hrtf));
    }

    Mode mode{Mode::microphones};
    std::vector<Microphone> microphones{Microphone{}};
    Pointer hrtf{};
};

struct App {
    inline float get_waveguide_sample_rate() const {
        return filter_frequency * oversample_ratio * 4;
    }

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("filter_frequency", filter_frequency),
                cereal::make_nvp("oversample_ratio", oversample_ratio),
                cereal::make_nvp("rays", rays),
                cereal::make_nvp("source", source),
                cereal::make_nvp("receiver", receiver),
                cereal::make_nvp("receiver_settings", receiver_settings));
    }

    float filter_frequency{500};
    float oversample_ratio{2};
    int rays{100000};
    glm::vec3 source{0};
    glm::vec3 receiver{0};
    ReceiverSettings receiver_settings;
};

}  // namespace model
