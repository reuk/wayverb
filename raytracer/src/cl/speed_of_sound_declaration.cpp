#include "raytracer/cl/speed_of_sound_declaration.h"

namespace cl_sources{

std::string speed_of_sound_declaration(double speed_of_sound) {
    return "const constant float SPEED_OF_SOUND = " +
           std::to_string(speed_of_sound) +
           ";\nconst constant float SECONDS_PER_METER = 1.0f / "
           "SPEED_OF_SOUND;\n";
}

}  // namespace cl_sources
