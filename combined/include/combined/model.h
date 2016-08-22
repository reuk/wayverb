#pragma once

#include "common/orientable.h"
#include "common/receiver_settings.h"

#include <vector>

namespace model {

struct SingleShot {
    float get_waveguide_sample_rate() const;

    float filter_frequency;
    float oversample_ratio;
    float speed_of_sound;
    size_t rays;
    glm::vec3 source;
    ReceiverSettings receiver_settings;
};

struct App {
    float get_waveguide_sample_rate() const;

    SingleShot get_single_shot(size_t input, size_t output) const;
    aligned::vector<SingleShot> get_all_input_output_combinations() const;

    float filter_frequency{500};
    float oversample_ratio{2};
    float speed_of_sound{340};
    size_t rays{100000};
    aligned::vector<glm::vec3> source{glm::vec3{0}};
    aligned::vector<ReceiverSettings> receiver_settings{ReceiverSettings{}};
};

}  // namespace model
