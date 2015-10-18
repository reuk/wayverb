#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "scene_data.h"

#define NUM_IMAGE_SOURCE 10
#define SPEED_OF_SOUND (340.0f)

//  These definitions MUST be kept up-to-date with the defs in the cl file.
//  It might make sense to nest them inside the Scene because I don't think
//  other classes will need the same data formats.

/// An impulse contains a volume, a time in seconds, and the direction from
/// which it came (useful for attenuation/hrtf stuff).
typedef struct  {
    VolumeType volume;
    cl_float3 position;
    cl_float time;
} __attribute__ ((aligned(8))) Impulse;

typedef struct  {
    VolumeType volume;
    cl_float time;
} __attribute__ ((aligned(8))) AttenuatedImpulse;

/// Each speaker has a (normalized-unit) direction, and a coefficient in the
/// range 0-1 which describes its polar pattern from omni to bidirectional.
typedef struct  {
    cl_float3 direction;
    cl_float coefficient;
} __attribute__ ((aligned(8))) Speaker;
