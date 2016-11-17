#include "combined/model/app.h"

#include "gtest/gtest.h"

using namespace wayverb::combined;
using namespace wayverb::core;

TEST(app_model, sources) {
    const auto quick_check = [](const auto& i) {
        ASSERT_EQ(i.connections(), 0);
        ASSERT_EQ(i[0]->connections(), 1);
    };

    model::vector<model::source, 1> a{model::source{geo::box{}}};
    quick_check(a);

    model::vector<model::source, 1> b{model::source{geo::box{}}};
    quick_check(b);

    a = b;
    quick_check(a);
    quick_check(b);

    a = std::move(b);
    quick_check(a);
    quick_check(b);

    model::sources sources{geo::box{glm::vec3{-1}, glm::vec3{1}}};
    ASSERT_EQ(sources.connections(), 0);
    ASSERT_EQ(sources[0]->connections(), 1);
}

TEST(app_model, copy_assignment) {
    bool called = false;

    model::microphone a;

    a.connect([&](auto&) { called = true; });

    model::microphone b;
    b.set_shape(1);
    b.set_orientation(orientation{compute_pointing(az_el{M_PI, 0.1})});

    a = b;

    ASSERT_TRUE(called);
}

TEST(app_model, app_model) {}
