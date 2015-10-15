#include "rayverb.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#ifdef DIAGNOSTIC
/// Print diagnostic json data to stdout.
void print_diagnostic
(   unsigned long nrays
,   unsigned long nreflections
,   const std::vector <Impulse> & reflections
,   const std::string & fname
);
#endif


// -1 <= z <= 1, -pi <= theta <= pi
/// Get a point on a unit sphere by azimuth and elevation.
cl_float3 spherePoint (float z, float theta);

//  Maybe we could have 'getDirections' a member of a class
//  which can be overridden.
//
//  Then 'Scene' can be a specialisation of this class.
//  Probably better if the scene takes a functor which can
//  generate rays as necessary.

/// Get a bunch of unit vectors which can be used as ray starting directions.
std::vector <cl_float3> getRandomDirections (unsigned long num);
std::vector <cl_float3> getUniformDirections (unsigned long num);
