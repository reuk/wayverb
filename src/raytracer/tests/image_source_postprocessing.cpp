#include "raytracer/attenuator.h"
#include "raytracer/image_source/exact.h"
#include "raytracer/image_source/get_direct.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "gtest/gtest.h"

using namespace wayverb::raytracer;
using namespace wayverb::core;

namespace {

TEST(image_source, binaural_amplitude) {
    constexpr auto absorption = 0.1f;
    constexpr auto surface = make_surface<simulation_bands>(absorption, 0);

    constexpr auto radius = 0.1f;

    struct distances final {
        double l;
        double r;
    };

    const auto get_distances = [&](const auto& source,
                                   const auto& receiver,
                                   const auto& pointing,
                                   const auto& voxelised) {
        constexpr glm::vec3 up{0, 1, 0};

        const auto impulse =
                image_source::get_direct(source, receiver, voxelised);

        if (!impulse) {
            throw std::runtime_error{
                    "Unable to generate direct impulse for some reason."};
        }

        const auto get_attenuated = [&](auto channel, const auto& impulse) {
            return attenuate(
                    attenuator::hrtf{
                            orientation{pointing, up}, channel, radius},
                    receiver,
                    impulse);
        };

        return distances{
                get_attenuated(attenuator::hrtf::channel::left, *impulse)
                        .distance,
                get_attenuated(attenuator::hrtf::channel::right, *impulse)
                        .distance};

    };

    {
        const geo::box box{glm::vec3{0, 0, 0}, glm::vec3{4, 3, 6}};

        const auto voxelised = make_voxelised_scene_data(
                geo::get_scene_data(box, surface), 5, 0.1f);

        {
            const auto d = get_distances(glm::vec3{2, 1.5, 1},
                                         glm::vec3{0.5, 1.5, 5},
                                         glm::vec3{0, 0, -1},
                                         voxelised);
            ASSERT_LT(d.r, d.l);
        }

        {
            const auto d = get_distances(glm::vec3{2, 1.5, 1},
                                         glm::vec3{3.5, 1.5, 5},
                                         glm::vec3{0, 0, -1},
                                         voxelised);
            ASSERT_LT(d.l, d.r);
        }

        {
            const auto d = get_distances(glm::vec3{2, 1.5, 1},
                                         glm::vec3{0.5, 1.5, 5},
                                         glm::vec3{0, 0, 1},
                                         voxelised);
            ASSERT_LT(d.l, d.r);
        }

        {
            const auto d = get_distances(glm::vec3{2, 1.5, 1},
                                         glm::vec3{3.5, 1.5, 5},
                                         glm::vec3{0, 0, 1},
                                         voxelised);
            ASSERT_LT(d.r, d.l);
        }
    }

    {
        const geo::box box{glm::vec3{-11, 0, -21}, glm::vec3{11, 3, 1}};
        const auto voxelised = make_voxelised_scene_data(
                geo::get_scene_data(box, surface), 5, 0.1f);

        {
            const auto d = get_distances(glm::vec3{0, 1.5, 0},
                                         glm::vec3{10, 1.5, -20},
                                         glm::vec3{0, 0, 1},
                                         voxelised);
            ASSERT_LT(d.r, d.l);
        }

        {
            const auto v = compute_pointing(az_el{M_PI, 0});
            ASSERT_TRUE(nearby(v, glm::vec3{0, 0, 1}, 0.00001));

            const auto d = get_distances(glm::vec3{0, 1.5, 0},
                                         glm::vec3{-10, 1.5, -20},
                                         v,
                                         voxelised);
            ASSERT_LT(d.l, d.r);
        }

        {
            const auto v = compute_pointing(az_el{0, 0});
            ASSERT_TRUE(nearby(v, glm::vec3{0, 0, -1}, 0.00001));

            const auto d = get_distances(glm::vec3{0, 1.5, 0},
                                         glm::vec3{-10, 1.5, -10},
                                         v,
                                         voxelised);
            ASSERT_LT(d.r, d.l);
        }
    }
}

}  // namespace
