#include "waveguide_config.h"

float WaveguideConfig::get_max_frequency() const {
    return get_filter_frequency() * get_oversample_ratio();
}

float & WaveguideConfig::get_oversample_ratio() {
    return oversample_ratio;
}

float & WaveguideConfig::get_filter_frequency() {
    return filter_frequency;
}

float WaveguideConfig::get_waveguide_sample_rate() const {
    return get_max_frequency() * 4;
}

float WaveguideConfig::get_divisions() const {
    return (get_speed_of_sound() * sqrt(3)) / get_waveguide_sample_rate();
}

float WaveguideConfig::get_oversample_ratio() const {
    return oversample_ratio;
}
float WaveguideConfig::get_filter_frequency() const {
    return filter_frequency;
}
