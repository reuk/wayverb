#include "app_config.h"

Vec3f & Config::get_source() {
    return source;
}

Vec3f & Config::get_mic() {
    return mic;
}

float & Config::get_speed_of_sound() {
    return speed_of_sound;
}

float & Config::get_output_sample_rate() {
    return sample_rate;
}

int & Config::get_bit_depth() {
    return bit_depth;
}

Vec3f Config::get_source() const {
    return source;
}
Vec3f Config::get_mic() const {
    return mic;
}
float Config::get_speed_of_sound() const {
    return speed_of_sound;
}
float Config::get_output_sample_rate() const {
    return sample_rate;
}
int Config::get_bit_depth() const {
    return bit_depth;
}
