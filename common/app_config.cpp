#include "app_config.h"

namespace config {

Vec3f& App::get_source() {
    return source;
}

Vec3f& App::get_mic() {
    return mic;
}

float& App::get_output_sample_rate() {
    return sample_rate;
}

int& App::get_bit_depth() {
    return bit_depth;
}

Vec3f App::get_source() const {
    return source;
}
Vec3f App::get_mic() const {
    return mic;
}
float App::get_output_sample_rate() const {
    return sample_rate;
}
int App::get_bit_depth() const {
    return bit_depth;
}
}
