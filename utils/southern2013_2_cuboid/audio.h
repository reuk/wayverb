#pragma once

#include "common/aligned/vector.h"

#include "audio_file/audio_file.h"

#include <string>

struct audio final {
    aligned::vector<float> data;
    double sample_rate;
    std::string prefix;
};

