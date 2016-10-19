#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"

#include "common/cl/iterator.h"
#include "common/geo/box.h"
#include "common/mixdown.h"
#include "common/model/parameters.h"
#include "common/pressure_intensity.h"

raytracer::results run_raytracer(const geo::box& box,
                                 const surface& surface,
                                 const model::parameters& params,
                                 size_t image_source_order);
