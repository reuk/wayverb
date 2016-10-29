#pragma once

#include "common/model/receiver_settings.h"
#include "common/orientable.h"

#include <vector>

namespace model {

struct SingleShot {
    float filter_frequency;
    float oversample_ratio;
    float speed_of_sound;
    size_t rays;
    glm::vec3 source;
    receiver_settings receiver_settings;
};

struct App {
    float filter_frequency{500};
    float oversample_ratio{2};
    float speed_of_sound{340};
    size_t rays{100000};
    aligned::vector<glm::vec3> source{glm::vec3{0}};
    aligned::vector<receiver_settings> receiver_settings{1};
};

SingleShot get_single_shot(const App& a, size_t input, size_t output);
aligned::vector<SingleShot> get_all_input_output_combinations(const App& a);

}  // namespace model
