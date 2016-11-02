#pragma once

#include <cstdlib>

namespace raytracer {

struct simulation_parameters final {
    /// The number of rays to use.
    /// More is better, but also will take longer.
    /// Use at least a few thousand.
    size_t rays;

    /// The raytracer uses an exact method to find early image sources.
    /// This method is more accurate than stochastic raytracing, but much slower
    /// and is not guaranteed to find all possible image sources, especially
    /// for higher reflection orders.
    /// This parameter controls the reflection order at which the model should
    /// switch from image-source contributions to stochastic specular
    /// contributions.
    /// Values from 0-5 are reasonable.
    size_t maximum_image_source_order;
};

}  // namespace raytracer
