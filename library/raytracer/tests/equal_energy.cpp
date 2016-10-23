#include "raytracer/image_source/get_direct.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflection_processor/stochastic_histogram.h"

#include "common/attenuator/microphone.h"

#include "gtest/gtest.h"

namespace {

template <typename Histogram, typename Attenuator>
void run_test(const Attenuator& attenuator) {
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

    const auto histogram =
            compute_summed_histogram(std::get<0>(*results), attenuator)
                    .histogram;

    const auto direct = raytracer::image_source::get_direct(
            params.source, params.receiver, voxelised);

    const auto direct_energy =
            attenuation(attenuator,
                        glm::normalize(params.source - params.receiver)) *
            intensity_for_distance(direct->distance);

    const auto histogram_energy =
            *std::find_if(begin(histogram), end(histogram), [](const auto& i) {
                return any(i);
            });

    using std::abs;
    const auto difference = abs(direct_energy - histogram_energy) * 2 /
                            (direct_energy + histogram_energy);

    //  energy values should be within 10% of one another
    ASSERT_TRUE(all(difference < 0.1));
}
}  // namespace

TEST(equal_energy, omni) {
    run_test<raytracer::reflection_processor::make_stochastic_histogram>(
            attenuator::null{});
}

TEST(equal_energy, directional) {
    run_test<raytracer::reflection_processor::make_directional_histogram>(
            attenuator::null{});
}

TEST(equal_energy, cardioid) {
    run_test<raytracer::reflection_processor::make_directional_histogram>(
            attenuator::microphone{glm::vec3{-1, 0, 0}, 0.5f});
}

TEST(equal_energy, hrtf) {
    run_test<raytracer::reflection_processor::make_directional_histogram>(
            attenuator::hrtf{glm::vec3{-1, 0, 0},
                             glm::vec3{0, 1, 0},
                             attenuator::hrtf::channel::left});
}
