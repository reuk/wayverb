#pragma once

/// TODO
/// A general filter for combining the raytracer and waveguide
/// outputs.
/// Parameters:
///     raytracer signal
///     raytracer sample_rate
///     waveguide signal
///     waveguide sample_rate
/// Returns:
///     combined, filtered signal

template <typename RayIt, typename WaveIt>
auto crossover_filter(frequency_domain::filter& filter,
                      RayIt b_ray,
                      RayIt e_ray,
                      double sr_ray,
                      WaveIt b_wave,
                      WaveIt e_wave,
                      double sr_wave) {
    throw std::runtime_error{"not implemented"};
}
