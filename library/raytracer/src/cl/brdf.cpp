#include "raytracer/cl/brdf.h"

namespace cl_sources {
const char* brdf{R"(
//  z range: -1 to 1
//  theta range: -pi to pi
float3 sphere_point(float z, float theta);
float3 sphere_point(float z, float theta) {
    const float t = sqrt(1 - z * z);
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

float3 lambert_scattering(float3 specular,
                          float3 surface_normal,
                          float3 random,
                          float d);
float3 lambert_scattering(float3 specular,
                          float3 surface_normal,
                          float3 random,
                          float d) {
    const float3 l = lambert_vector(surface_normal, random);
    return normalize((l * d) + (specular * (1 - d)));
}

//----------------------------------------------------------------------------//

/*
//  Taken from
//  http://file.scirp.org/pdf/OJA_2015122513452619.pdf
//  but with some modifications so that rays only radiate out in a hemisphere
//  instead of a sphere.

//  y: angle between specular and outgoing vectors (radians)
//  d: the directionality coefficient of the surface
//
//  TODO we might need to adjust the magnitude to correct for not-radiating-in-
//  all-directions
float get_frac(float numerator, float denominator);
float get_frac(float numerator, float denominator) {
    return denominator ? numerator / denominator : 0;
}

float brdf_mag(float y, float d);
float brdf_mag(float y, float d) {
    const float y_sq = y * y;
    const float one_minus_d_sq = pow(1 - d, 2);
    const float numerator = 2 * one_minus_d_sq * y_sq + 2 * d - 1;

    const float to_sqrt = max(one_minus_d_sq * y_sq + 2 * d - 1, 0.0f);

    if (0.5 <= d) {
        const float denominator = 4 * M_PI * d * sqrt(to_sqrt);
        const float extra = ((1 - d) * y) / (2 * M_PI * d);
        return get_frac(numerator, denominator) + extra;
    }

    const float denominator = 2 * M_PI * d * sqrt(to_sqrt);
    return get_frac(numerator, denominator);
}

//  specular: the specular reflection direction (unit vector)
//  outgoing: the actual direction of the outgoing reflection (unit vector)
//  d: the directionality coefficient of the surface
float brdf_mag_for_outgoing(float3 specular, float3 outgoing, float d);
float brdf_mag_for_outgoing(float3 specular, float3 outgoing, float d) {
    return brdf_mag(dot(specular, outgoing), d);
}

bands_type brdf_mags_for_outgoing(float3 specular,
                                   float3 outgoing,
                                   bands_type d);
bands_type brdf_mags_for_outgoing(float3 specular,
                                   float3 outgoing,
                                   bands_type d) {
    return (bands_type)(brdf_mag_for_outgoing(specular, outgoing, d.s0),
                         brdf_mag_for_outgoing(specular, outgoing, d.s1),
                         brdf_mag_for_outgoing(specular, outgoing, d.s2),
                         brdf_mag_for_outgoing(specular, outgoing, d.s3),
                         brdf_mag_for_outgoing(specular, outgoing, d.s4),
                         brdf_mag_for_outgoing(specular, outgoing, d.s5),
                         brdf_mag_for_outgoing(specular, outgoing, d.s6),
                         brdf_mag_for_outgoing(specular, outgoing, d.s7));
}
*/

//----------------------------------------------------------------------------//

float mean(bands_type v);
float mean(bands_type v) {
    return (v.s0 + v.s1 + v.s2 + v.s3 + v.s4 + v.s5 + v.s6 + v.s7) / 8;
}
)"};
}  // namespace cl_sources
