#pragma once

//  please only include in .cpp files

#include <string>

namespace cl_sources {
const std::string brdf(R"(

typedef struct {
    float3 direction;
    float magnitude;
} BRDFOutput;

//  z range: -1 to 1
//  theta range: -pi to pi
float3 sphere_point(float z, float theta);
float3 sphere_point(float z, float theta) {
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

VolumeType brdf_mags_for_outgoing(float3 specular, float3 outgoing, VolumeType d);
VolumeType brdf_mags_for_outgoing(float3 specular, float3 outgoing, VolumeType d) {
    return (VolumeType)(
        brdf_mag_for_outgoing(specular, outgoing, d.s0),
        brdf_mag_for_outgoing(specular, outgoing, d.s1),
        brdf_mag_for_outgoing(specular, outgoing, d.s2),
        brdf_mag_for_outgoing(specular, outgoing, d.s3),
        brdf_mag_for_outgoing(specular, outgoing, d.s4),
        brdf_mag_for_outgoing(specular, outgoing, d.s5),
        brdf_mag_for_outgoing(specular, outgoing, d.s6),
        brdf_mag_for_outgoing(specular, outgoing, d.s7));
}

float mean(VolumeType v);
float mean(VolumeType v) {
    return (v.s0 + v.s1 + v.s2 + v.s3 + v.s4 + v.s5 + v.s6 + v.s7) / 8;
}

//----------------------------------------------------------------------------//

)");
}  // namespace cl_sources
