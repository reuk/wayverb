#include "core/pressure_intensity.h"

double intensity_for_distance(double distance) {
    return 1 / (4 * M_PI * distance * distance);
}

double pressure_for_distance(double distance, double acoustic_impedance) {
    return std::sqrt(acoustic_impedance / (4 * M_PI)) / distance;
}
