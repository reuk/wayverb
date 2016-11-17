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

    {
        model::shared_value<model::persistent> p{
                model::persistent{geo::box{glm::vec3{-1}, glm::vec3{1}}}};

        bool source_changed = false;
        (*p->sources())[0]->connect([&](auto&) { source_changed = true; });

        auto q = std::move(p);

        ASSERT_FALSE(source_changed);

        (*q->sources())[0]->position()->set({0.5, 0.5, 0.5});

        ASSERT_TRUE(source_changed);

        source_changed = false;

        model::source new_source{geo::box{glm::vec3{-2}, glm::vec3{2}}};
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
