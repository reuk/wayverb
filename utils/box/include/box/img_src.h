#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/postprocess.h"

#include "common/cl/iterator.h"
#include "common/geo/box.h"
#include "common/mixdown.h"
#include "common/pressure_intensity.h"

aligned::vector<impulse<8>> run_exact_img_src(const geo::box& box,
                                              float absorption,
                                              const glm::vec3& source,
                                              const glm::vec3& receiver,
                                              float speed_of_sound,
                                              float acoustic_impedance,
                                              float simulation_time);

aligned::vector<impulse<8>> run_fast_img_src(const geo::box& box,
                                             float absorption,
                                             const glm::vec3& source,
                                             const glm::vec3& receiver,
                                             float acoustic_impedance);
