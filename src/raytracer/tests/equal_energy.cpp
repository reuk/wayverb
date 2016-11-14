#include "raytracer/image_source/get_direct.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflection_processor/stochastic_histogram.h"

#include "core/attenuator/microphone.h"

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

    const auto histogram_sample_rate = 1000.0f;
    const auto callbacks =
            std::make_tuple(Histogram{1.0f, histogram_sample_rate, 0});

    const auto directions = get_random_directions(1 << 16);
    const auto results = run(begin(directions),
                             end(directions),
                             compute_context{},
                             voxelised,
                             source,
                             receiver,
                             environment,
                             true,
                             [](auto, auto) {},
                             callbacks);

    ASSERT_TRUE(results);

    const auto direct_energy = [&] {
        const auto direct =
                image_source::get_direct(source, receiver, voxelised);
        const auto att = attenuation(
                attenuator, glm::normalize(source - receiver));
        return att * att * intensity_for_distance(direct->distance);
    }();

    const auto histogram_energy = [&] {
        const auto histogram =
                compute_summed_histogram(std::get<0>(*results), attenuator)
                        .histogram;
        const auto histogram_bin = glm::distance(source, receiver) *
                                   histogram_sample_rate /
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
                attenuator::microphone{orientable{dir}, 0.5f});
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
            attenuator::hrtf{orientable{{-1, 0, 0}, {0, 1, 0}},
                             attenuator::hrtf::channel::left});
}

}  // namespace
