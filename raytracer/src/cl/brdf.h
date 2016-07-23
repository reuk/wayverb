#pragma once

//  please only include in .cpp files

#include <string>

namespace cl_sources {
const std::string brdf(R"(

//  theta range: -pi to pi
//  z range: -1 to 1
float3 unit_vector(float theta, float z);
float3 unit_vector(float theta, float z) {
    float t = sqrt(1 - z * z);
    return (float3)(t * cos(theta), z, t * sin(theta));
}

//----------------------------------------------------------------------------//

//  surface_normal: the direction of the surface normal (unit vector)
//  random: a random vector (unit vector)
//
//  returns: a random vector in a hemisphere oriented according to the surface
//           normal
float3 lambert_vector(float3 surface_normal, float3 random);
float3 lambert_vector(float3 surface_normal, float3 random) {
    return random * signbit(dot(random, surface_normal));
}

float3 lambert_scattering(float3 specular, float3 surface_normal, float3 random, float d);
float3 lambert_scattering(float3 specular, float3 surface_normal, float3 random, float d) {
    //  TODO what if the surface normal is inverted?
    float3 l = lambert_vector(surface_normal, random);

    return normalize((l * d) + (specular * (1 - d)));
}

//----------------------------------------------------------------------------//

//  Taken from
//  http://file.scirp.org/pdf/OJA_2015122513452619.pdf
//  but with some modifications so that rays only radiate out in a hemisphere
//  instead of a sphere.

//  y: angle between specular and outgoing vectors (radians)
//  d: the directionality coefficient of the surface
//
//  TODO we might need to adjust the magnitude to correct for not-radiating-in-
//  all-directions
float brdf_mag(float y, float d);
float brdf_mag(float y, float d) {
    float common = pow(1 - d, 2) * pow(y, 2) + 2 * d - 1;
    float numerator = 2 * common;
    if (d < 0.5) {
        float denominator = 2 * M_PI_F * d * sqrt(common);
        return numerator / denominator;
    }
    float denominator = 4 * M_PI_F * d * sqrt(common);
    float extra = ((1 - d) * y) / (2 * M_PI_F * d);
    return (numerator / denominator) + extra;
}

//  specular: the specular reflection direction (unit vector)
//  outgoing: the actual direction of the outgoing reflection (unit vector)
//  d: the directionality coefficient of the surface
float brdf_mag_for_outgoing(float3 specular, float3 outgoing, float d);
float brdf_mag_for_outgoing(float3 specular, float3 outgoing, float d) {
    //  find angle between specular and outgoing vectors
    float y = dot(specular, outgoing);

    //  get the magnitude of the 'diffuse' reflection
    //
    //  TODO we might need to adjust the magnitude to correct for not-radiating-
    //  in-all-directions
    return brdf_mag(y, d);
}

//  specular: the specular reflection direction (unit vector)
//  surface_normal: the direction of the surface normal (unit vector)
//  random: a random vector (unit vector)
//  d: the directionality coefficient of the surface
BRDFOutput brdf(float3 specular, float3 surface_normal, float3 random, float d);
BRDFOutput brdf(float3 specular, float3 surface_normal, float3 random, float d) {
    float3 outgoing = lambert_scattering(specular, surface_normal, random, d);

    //  get the magnitude of the 'diffuse' reflection
    float p = brdf_mag_for_outgoing(specular, outgoing, d);
    return (BRDFOutput){outgoing, p};
}

//----------------------------------------------------------------------------//

)");
}  // namespace cl_sources
