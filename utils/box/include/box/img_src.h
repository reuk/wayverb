#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/postprocess.h"

#include "common/cl/iterator.h"
#include "common/geo/box.h"
#include "common/mixdown.h"
#include "common/model/parameters.h"
#include "common/pressure_intensity.h"

aligned::vector<impulse<8>> run_exact_img_src(const geo::box& box,
                                              float absorption,
                                              const model::parameters& params,
                                              float simulation_time,
                                              bool flip_phase);

aligned::vector<impulse<8>> run_fast_img_src(const geo::box& box,
                                             float absorption,
                                             const model::parameters& params,
                                             bool flip_phase);
