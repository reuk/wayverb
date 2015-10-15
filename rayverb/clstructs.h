#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#define NUM_IMAGE_SOURCE 10
#define SPEED_OF_SOUND (340.0f)

//  These definitions MUST be kept up-to-date with the defs in the cl file.
//  It might make sense to nest them inside the Scene because I don't think
//  other classes will need the same data formats.

/// Type used for storing multiband volumes.
/// Higher values of 'x' in cl_floatx = higher numbers of parallel bands.
using VolumeType = cl_float8;

/// A Triangle contains an offset into an array of Surface, and three offsets
/// into an array of cl_float3.
typedef struct {
    cl_ulong surface;
    cl_ulong v0;
    cl_ulong v1;
    cl_ulong v2;
} __attribute__ ((aligned(8))) Triangle;

/// Surfaces describe their specular and diffuse coefficients per-band.
typedef struct  {
    VolumeType specular;
    VolumeType diffuse;
} __attribute__ ((aligned(8))) Surface;

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
