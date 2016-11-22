#include "combined/model/app.h"

#include "gtest/gtest.h"

using namespace wayverb::combined;
using namespace wayverb::core;

TEST(app_model, sources) {
    const auto quick_check = [](const auto& i) {
        ASSERT_EQ(i.connections(), 0);
        ASSERT_EQ(i[0].item()->connections(), 1);
    };

    model::sources a{};
    quick_check(a);

    model::sources b{};
    quick_check(b);

    a = b;
    quick_check(a);
    quick_check(b);

    a = std::move(b);
    quick_check(a);
    quick_check(b);

    model::sources sources{};
    ASSERT_EQ(sources.connections(), 0);
    ASSERT_EQ(sources[0].item()->connections(), 1);

    {
        model::persistent p;

        bool source_changed = false;
        (*p.sources().item())[0].item()->connect(
                [&](auto&) { source_changed = true; });

        auto q = p;

        ASSERT_FALSE(source_changed);

        (*p.sources().item())[0].item()->set_position({0.5, 0.5, 0.5});

        ASSERT_TRUE(source_changed);

        source_changed = false;

        model::source new_source{};
        *(*p.sources().item())[0].item() = new_source;

        ASSERT_TRUE(source_changed);
    }
}

template <typename T>
void test_copy_assignment(const T& b) {
    auto called = false;

    T a;
    a.connect([&](auto&) { called = true; });

    a = b;

    ASSERT_TRUE(called);
}

TEST(app_model, copy_assignment) {
    util::for_each_params([](auto i) { test_copy_assignment(i); },
                          model::capsule{},
                          model::hover_state{},
                          model::hrtf{},
                          model::material{},
                          model::microphone{},
                          model::output{},
                          model::raytracer{},
                          model::receiver{},
                          model::source{},
                          model::single_band_waveguide{},
                          model::multiple_band_waveguide{},
                          model::waveguide{});
}
