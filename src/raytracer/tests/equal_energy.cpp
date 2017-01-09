#include "raytracer/hit_rate.h"
#include "raytracer/image_source/get_direct.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflection_processor/stochastic_histogram.h"

#include "core/attenuator/microphone.h"
#include "core/reverb_time.h"

#include "gtest/gtest.h"

using namespace wayverb::raytracer;
using namespace wayverb::core;

namespace {

template <typename Histogram, typename Attenuator>
void run_test(const Attenuator& attenuator) {
    const geo::box box{glm::vec3{-4}, glm::vec3{4}};
    constexpr auto absorption = 0.1;
    constexpr auto scattering = 0.1;

    constexpr glm::vec3 source{-2, 0, 0}, receiver{2, 0, 0};

    constexpr wayverb::core::environment environment{};

    const auto voxelised = make_voxelised_scene_data(
            geo::get_scene_data(
                    box,
                    make_surface<simulation_bands>(absorption, scattering)),
            2,
            0.1f);

    const auto params = wayverb::raytracer::make_simulation_parameters(
            100,
            1.0,
            environment.speed_of_sound,
            1000.0,
            estimate_room_volume(voxelised.get_scene_data()),
            0);

    std::default_random_engine engine{std::random_device{}()};

    const auto results =
            run(make_random_direction_generator_iterator(0, engine),
                make_random_direction_generator_iterator(params.rays, engine),
                compute_context{},
                voxelised,
                source,
                receiver,
                environment,
                true,
                [](auto i, auto tot) {
                    std::cout << "chunk " << i << " of " << tot << '\n';
                },
                std::make_tuple(Histogram(params.rays,
                                          params.maximum_image_source_order,
                                          params.receiver_radius,
                                          params.histogram_sample_rate)));

    ASSERT_TRUE(results);

    const auto direct_energy = [&] {
        const auto direct =
                image_source::get_direct(source, receiver, voxelised);
        const auto att =
                attenuation(attenuator, glm::normalize(source - receiver));
        return att * att * intensity_for_distance(direct->distance);
    }();

    const auto histogram_energy = [&] {
        const auto histogram =
                compute_summed_histogram(std::get<0>(*results), attenuator)
                        .histogram;
        const auto histogram_bin = glm::distance(source, receiver) *
                                   params.histogram_sample_rate /
                                   environment.speed_of_sound;
        return histogram[histogram_bin];
    }();

    using std::abs;
    const auto denominator = abs(abs(direct_energy) + abs(histogram_energy));
    if (all(denominator)) {
        const auto difference =
                abs(abs(direct_energy) - abs(histogram_energy)) * 2 /
                denominator;

        //  energy values should be within 10% of one another
        ASSERT_TRUE(all(difference < 0.1));
    }
}

TEST(equal_energy, omni) {
    run_test<reflection_processor::make_stochastic_histogram>(
            attenuator::null{});
}

TEST(equal_energy, directional) {
    run_test<reflection_processor::make_directional_histogram>(
            attenuator::null{});
}

TEST(equal_energy, cardioid) {
    const auto go = [](auto dir) {
        run_test<reflection_processor::make_directional_histogram>(
                attenuator::microphone{orientation{dir}, 0.5f});
    };
    go(glm::vec3{-1, 0, 0});
    go(glm::vec3{1, 0, 0});
    go(glm::vec3{0, -1, 0});
    go(glm::vec3{0, 1, 0});
    go(glm::vec3{0, 0, -1});
    go(glm::vec3{0, 0, 1});
}

TEST(equal_energy, hrtf) {
    run_test<reflection_processor::make_directional_histogram>(
            attenuator::hrtf{orientation{{-1, 0, 0}, {0, 1, 0}},
                             attenuator::hrtf::channel::left});
}

}  // namespace
