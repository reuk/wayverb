#include "combined/model/app.h"

#include "gtest/gtest.h"

TEST(app_model, sources) {
    const auto quick_check = [] (const auto& i) {
        ASSERT_EQ(i.connections(), 0);
        ASSERT_EQ(i[0].connections(), 1);
    };

    wayverb::combined::model::vector<wayverb::combined::model::source, 1> a;
    quick_check(a);

    wayverb::combined::model::vector<wayverb::combined::model::source, 1> b;
    quick_check(b);

    a = b;
    quick_check(a);
    quick_check(b);

    a = std::move(b);
    quick_check(a);
    quick_check(b);

    wayverb::combined::model::sources sources{wayverb::core::geo::box{glm::vec3{-1}, glm::vec3{1}}};
    ASSERT_EQ(sources.connections(), 0);
    ASSERT_EQ(sources.data().connections(), 1);
    ASSERT_EQ(sources[0].connections(), 1);
}

TEST(app_model, copy_assignment) {
    bool called = false;

    wayverb::combined::model::microphone a;

    a.connect([&](auto&) { called = true; });

    wayverb::combined::model::microphone b;
    b.set_shape(1);
    b.set_orientation(wayverb::core::orientation{
            compute_pointing(wayverb::core::az_el{M_PI, 0.1})});

    a = b;

    ASSERT_TRUE(called);
}

TEST(app_model, app_model) {

}
