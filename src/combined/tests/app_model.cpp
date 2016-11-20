#include "combined/model/app.h"

#include "gtest/gtest.h"

using namespace wayverb::combined;
using namespace wayverb::core;

TEST(app_model, sources) {
    const auto quick_check = [](const auto& i) {
        ASSERT_EQ(i.connections(), 0);
        ASSERT_EQ(i[0]->connections(), 1);
    };

    model::vector<model::source, 1> a{model::source{}};
    quick_check(a);

    model::vector<model::source, 1> b{model::source{}};
    quick_check(b);

    a = b;
    quick_check(a);
    quick_check(b);

    a = std::move(b);
    quick_check(a);
    quick_check(b);

    model::sources sources{};
    ASSERT_EQ(sources.connections(), 0);
    ASSERT_EQ(sources[0]->connections(), 1);

    {
        model::shared_value<model::persistent> p{
                model::persistent{}};

        bool source_changed = false;
        (*p->sources())[0]->connect([&](auto&) { source_changed = true; });

        auto q = std::move(p);

        ASSERT_FALSE(source_changed);

        (*q->sources())[0]->set_position({0.5, 0.5, 0.5});

        ASSERT_TRUE(source_changed);

        source_changed = false;

        model::source new_source{};
        (*q->sources())[0] = new_source;

        ASSERT_TRUE(source_changed);
    }
}

TEST(app_model, copy_assignment) {
    bool called = false;

    model::shared_value<model::microphone> a;

    a->connect([&](auto&) { called = true; });

    model::shared_value<model::microphone> b;
    b->set_shape(1);
    b->set_orientation(orientation{compute_pointing(az_el{M_PI, 0.1})});

    a = b;

    ASSERT_TRUE(called);
}

TEST(app_model, app_model) {}
