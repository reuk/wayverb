#include "raytracer/image_source/exact.h"
#include "raytracer/image_source/get_direct.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "gtest/gtest.h"

using namespace wayverb::raytracer;
using namespace wayverb::core;

namespace {

template <size_t channels>
bool approximately_matches(const impulse<channels>& a,
                           const impulse<channels>& b) {
    const auto near = [](auto a, auto b) { return nearby(a, b, 0.0001); };
    for (auto i = 0ul; i != channels; ++i) {
        if (!near(a.volume.s[i], b.volume.s[0])) {
            return false;
        }
    }
    return near(a.distance, b.distance) &&
           near(a.position.s[0], b.position.s[0]) &&
           near(a.position.s[1], b.position.s[1]) &&
           near(a.position.s[2], b.position.s[2]);
}

void image_source_test() {
    const geo::box box{glm::vec3{0, 0, 0}, glm::vec3{4, 3, 6}};

    std::default_random_engine engine{std::random_device{}()};

    std::uniform_real_distribution<float> x_dist{box.get_min().x,
                                                 box.get_max().x};
    std::uniform_real_distribution<float> y_dist{box.get_min().y,
                                                 box.get_max().y};
    std::uniform_real_distribution<float> z_dist{box.get_min().z,
                                                 box.get_max().z};

    const glm::vec3 source{x_dist(engine), y_dist(engine), z_dist(engine)};
    const glm::vec3 receiver{x_dist(engine), y_dist(engine), z_dist(engine)};
    constexpr wayverb::core::environment environment{};

    constexpr auto absorption = 0.1f;
    constexpr auto surface = make_surface<simulation_bands>(absorption, 0);

    auto exact_impulses = image_source::find_impulses(
            box, source, receiver, surface.absorption, 10);
    for (auto& it : exact_impulses) {
        it.volume *= wayverb::core::pressure_for_distance(
                it.distance, environment.acoustic_impedance);
    }

    const auto check_distances = [&](const auto& range) {
        for (const auto& imp : range) {
            ASSERT_NEAR(glm::distance(receiver, to_vec3{}(imp.position)),
                        imp.distance,
                        0.0001);
        }
    };

    check_distances(exact_impulses);

    const auto voxelised = make_voxelised_scene_data(
            geo::get_scene_data(box, surface), 5, 0.1f);

    auto inexact_impulses = image_source::run(
            make_random_direction_generator_iterator(0, engine),
            make_random_direction_generator_iterator(10000, engine),
            compute_context{},
            voxelised,
            source,
            receiver,
            environment);

    check_distances(inexact_impulses);

    ASSERT_TRUE(inexact_impulses.size() > 1);

    const auto distance_comparator = [](const auto& a, const auto& b) {
        return a.distance < b.distance;
    };

    for (const auto& i : {&exact_impulses, &inexact_impulses}) {
        std::sort(i->begin(), i->end(), distance_comparator);
    }

    for (const auto& i : exact_impulses) {
        const auto possible_lower = std::lower_bound(inexact_impulses.begin(),
                                                     inexact_impulses.end(),
                                                     i,
                                                     distance_comparator);
        const auto lower = possible_lower == inexact_impulses.begin()
                                   ? possible_lower
                                   : possible_lower - 1;
        const auto possible_upper = std::upper_bound(inexact_impulses.begin(),
                                                     inexact_impulses.end(),
                                                     i,
                                                     distance_comparator);
        const auto upper = possible_upper == inexact_impulses.end()
                                   ? possible_upper
                                   : possible_upper + 1;
        util::aligned::vector<impulse<8>> possibilities(lower, upper);
        if (std::none_of(lower, upper, [&](const auto& x) {
                return approximately_matches(i, x);
            })) {
            throw std::runtime_error{"No approximate matches."};
        }
    }
}

TEST(image_source, fast_pressure) { ASSERT_NO_THROW(image_source_test()); }

}  // namespace
