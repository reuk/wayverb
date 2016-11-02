#include "waveguide/arbitrary_magnitude_filter.h"
#include "waveguide/stable.h"

#include "gtest/gtest.h"

#include <random>

using namespace wayverb::waveguide;

TEST(arbitrary_magnitude_filter, stable) {
    const auto test = [](const auto& env) {
        ASSERT_TRUE(is_stable(arbitrary_magnitude_filter<6>(env).a));
    };

    test(frequency_domain_envelope{});

    {
        auto env = frequency_domain_envelope{};

        env.insert(frequency_domain_envelope::point{0, 0});
        test(env);

        env.insert(frequency_domain_envelope::point{0.5, 1});
        test(env);

        env.insert(frequency_domain_envelope::point{0.49, 0});
        test(env);

        env.insert(frequency_domain_envelope::point{0.51, 0});
        test(env);
    }

    auto engine = std::default_random_engine{std::random_device{}()};
    auto dist = std::uniform_real_distribution<float>{0, 1};

    for (auto i = 0; i != 1000; ++i) {
        auto env = frequency_domain_envelope{};
        for (auto j = 0; j != 100; ++j) {
            env.insert(frequency_domain_envelope::point{dist(engine),
                                                        dist(engine)});
        }
        test(env);
    }
}
