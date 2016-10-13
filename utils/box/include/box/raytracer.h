#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/postprocess.h"

#include "common/cl/iterator.h"
#include "common/geo/box.h"
#include "common/mixdown.h"
#include "common/pressure_intensity.h"

aligned::vector<impulse<8>> run_raytracer(const geo::box& box,
                                          float absorption,
                                          float diffusion,
                                          const glm::vec3& source,
                                          const glm::vec3& receiver,
                                          float acoustic_impedance,
                                          bool flip_phase);
