#include "common/pressure_intensity.h"

float intensity_for_distance(float distance) {
    return 1 / (4 * M_PI * distance * distance);
}

float pressure_for_distance(float distance, float acoustic_impedance) {
    return std::sqrt(acoustic_impedance / (4 * M_PI)) / distance;
}
