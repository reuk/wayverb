#pragma once

#include "raytracer_config.h"
#include "waveguide_config.h"

namespace config {

class Combined : public Waveguide, public Raytracer {};

}  // namespace config
