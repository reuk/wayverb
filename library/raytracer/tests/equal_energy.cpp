#include "raytracer/histogram.h"
#include "raytracer/image_source/get_direct.h"
#include "raytracer/reflector.h"
#include "raytracer/stochastic/finder.h"

#include "gtest/gtest.h"

TEST(equal_energy, img_src_and_stochastic) {
    const geo::box box{glm::vec3{-4}, glm::vec3{4}};
    constexpr auto absorption = 0.1;
    constexpr auto scattering = 0.1;

    constexpr model::parameters params{glm::vec3{-2, 0, 0}, glm::vec3{2, 0, 0}};

    const auto voxelised = make_voxelised_scene_data(
            geo::get_scene_data(
                    box,
                    make_surface<simulation_bands>(absorption, scattering)),
            2,
            0.1f);

    const compute_context cc{};

    const auto directions = get_random_directions(1 << 16);

    const scene_buffers buffers{cc.context, voxelised};

    constexpr auto receiver_radius = 1.5f;
    constexpr auto histogram_sr = 1000.0f;

    raytracer::stochastic::finder finder{
            cc, params, receiver_radius, directions.size()};
    aligned::vector<bands_type> histogram;

    const auto make_ray_iterator = [&](auto it) {
        return make_mapping_iterator_adapter(std::move(it), [&](const auto& i) {
            return geo::ray{params.source, i};
        });
    };

    raytracer::reflector ref{cc,
                             params.receiver,
                             make_ray_iterator(begin(directions)),
                             make_ray_iterator(end(directions))};

    const auto reflections = ref.run_step(buffers);

    const auto output =
            finder.process(begin(reflections), end(reflections), buffers);
    const auto to_histogram = [&](auto& in) {
        const auto make_iterator = [&](auto it) {
            return raytracer::make_histogram_iterator(std::move(it),
                                                      params.speed_of_sound);
        };
        constexpr auto max_time = 60.0;
        incremental_histogram(histogram,
                              make_iterator(begin(in)),
                              make_iterator(end(in)),
                              histogram_sr,
                              max_time,
                              raytracer::dirac_sum_functor{});
    };
    to_histogram(output.stochastic);
    to_histogram(output.specular);

    const auto direct = raytracer::image_source::get_direct(
            params.source, params.receiver, voxelised);

    const auto direct_energy = intensity_for_distance(direct->distance);
    std::cout << "direct energy: " << direct_energy << '\n';

    const auto histogram_nonzero =
            std::find_if(begin(histogram), end(histogram), [](const auto& i) {
                return any(i);
            });

    const auto histogram_energy = histogram_nonzero->s[0];
    std::cout << "histogram energy: " << histogram_energy << '\n';

    ASSERT_NEAR(direct_energy, histogram_energy, 0.01);
}
