#include "raytracer/image_source/get_direct.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflection_processor/stochastic_histogram.h"

#include "gtest/gtest.h"

namespace {

const auto& sum_histogram(
        const raytracer::stochastic::energy_histogram& histogram) {
    return histogram;
}

template <size_t Az, size_t El>
auto sum_histogram(
        const raytracer::stochastic::directional_energy_histogram<Az, El>&
                histogram) {
    return raytracer::stochastic::sum_directional_histogram(histogram);
}

template <typename Histogram>
void run_test() {
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

    const auto callbacks = std::make_tuple(Histogram{1.0f, 1000.0f, 0});

    const auto directions = get_random_directions(1 << 16);
    const auto results = raytracer::run(begin(directions),
                                        end(directions),
                                        compute_context{},
                                        voxelised,
                                        params,
                                        true,
                                        [](auto i, auto steps) {},
                                        callbacks);

    ASSERT_TRUE(results);

    const auto histogram = sum_histogram(std::get<0>(*results)).histogram;

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

    ASSERT_LT(std::abs(direct_energy - histogram_energy) * 2 /
                      (direct_energy + histogram_energy),
              0.1);  //  energy values should be within 10% of one another
}
}  // namespace

TEST(equal_energy, omni) {
    run_test<raytracer::reflection_processor::make_stochastic_histogram>();
}

TEST(equal_energy, directional) {
    run_test<raytracer::reflection_processor::make_directional_histogram>();
}
