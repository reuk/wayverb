#include "combined/model/app.h"

#include "core/serialize/attenuators.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/memory.hpp"

#include "gtest/gtest.h"

using namespace wayverb::combined;
using namespace wayverb::core;

namespace {

template <typename T>
constexpr bool value_eq(const T& a, const T& b) {
    return a == b;
}

template <typename T>
constexpr bool value_eq(const std::unique_ptr<T>& a,
                        const std::unique_ptr<T>& b) {
    return *a == *b;
}

template <typename T>
constexpr bool value_eq(const std::shared_ptr<T>& a,
                        const std::shared_ptr<T>& b) {
    return *a == *b;
}

}  // namespace

template <typename T>
void round_trip(T init) {
    std::stringstream serialized;
    {
        cereal::JSONOutputArchive archive(serialized);
        archive(init);
    }

    T deserialized;

    {
        cereal::JSONInputArchive archive(serialized);
        archive(deserialized);
    }

    ASSERT_TRUE(value_eq(deserialized, init));

    std::stringstream reserialized;
    {
        cereal::JSONOutputArchive oArchive(reserialized);
        oArchive(deserialized);
    }

    ASSERT_EQ(serialized.str(), reserialized.str());
}

TEST(round_trip, microphone) {
    round_trip(model::microphone{});
    round_trip(model::microphone{orientation{{-1, 0, 0}}, 0.5});
    round_trip(model::microphone{orientation{{0, 2, 0}}, 0.75});
}

TEST(round_trip, hrtf) {
    round_trip(model::hrtf{});
    round_trip(model::hrtf{orientation{{2, 2, 0}},
                           attenuator::hrtf::channel::left});
    round_trip(model::hrtf{orientation{{0, -2, 1}},
                           attenuator::hrtf::channel::right});
}

TEST(round_trip, capsule) {
    round_trip(model::capsule{});
    round_trip(model::capsule{"some capsule name",
                              model::capsule::mode::hrtf,
                              model::microphone{orientation{{1, 1, 1}}, 0.2},
                              model::hrtf{orientation{{-1, -1, -1}},
                                          attenuator::hrtf::channel::left}});
    round_trip(model::capsule{"hrtf",
                              model::hrtf{orientation{{2, 3, 4}},
                                          attenuator::hrtf::channel::right}});
    round_trip(model::capsule{
            "mic", model::microphone{orientation{{-2, -3, -4}}, 0.8}});
}

TEST(round_trip, material) {
    round_trip(model::material{});
    round_trip(model::material{"some interesting material name",
                               make_surface<simulation_bands>(1.0, 0.9)});
    round_trip(model::material{"blah blah blah blah blah",
                               make_surface<simulation_bands>(-100.0, -20.0)});
}

TEST(round_trip, source) {
    round_trip(model::source{});
    round_trip(model::source{"xyz foo bar", {6, 7, 8}});
    round_trip(model::source{"gravitas", {-1000, -1000, -2000}});
}

TEST(round_trip, receiver) {
    round_trip(model::receiver{});
    round_trip(model::receiver{"baz", {4, 5, 6}, orientation{{0, 3, 4}}});
    round_trip(
            model::receiver{"elephant", {-2, 3, -4}, orientation{{0, 1, 0}}});
}

TEST(round_trip, raytracer) {
    round_trip(model::raytracer{});
    round_trip(model::raytracer{model::raytracer::ray_number::r1e6, 10});
    round_trip(model::raytracer{model::raytracer::ray_number::r1e3, 0});
}

TEST(round_trip, single_band_waveguide) {
    round_trip(model::single_band_waveguide{});
    round_trip(model::single_band_waveguide{1, 1});
    round_trip(model::single_band_waveguide{2, 2});
}

TEST(round_trip, multiple_band_waveguide) {
    round_trip(model::multiple_band_waveguide{});
    round_trip(model::multiple_band_waveguide{10, 2, 4});
    round_trip(model::multiple_band_waveguide{0, 10, 100});
}

TEST(round_trip, waveguide) {
    round_trip(model::waveguide{});
    round_trip(model::waveguide{model::waveguide::mode::single,
                                model::single_band_waveguide{100, 0.3},
                                model::multiple_band_waveguide{4, 500, 0.4}});
    round_trip(model::waveguide{model::single_band_waveguide{400, 0.2}});
    round_trip(model::waveguide{model::multiple_band_waveguide{5, 401, 0.21}});
}

TEST(round_trip, persistent) { 
    round_trip(model::persistent{}); 

    model::persistent p;

    round_trip(model::persistent{}); 
}
