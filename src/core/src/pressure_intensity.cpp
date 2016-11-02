#include "core/pressure_intensity.h"

namespace wayverb {
namespace core {

double intensity_for_distance(double distance) {
    return 1 / (4 * M_PI * distance * distance);
}

double pressure_for_distance(double distance, double acoustic_impedance) {
    return std::sqrt(acoustic_impedance / (4 * M_PI)) / distance;
}

}  // namespace core
}  // namespace wayverb
