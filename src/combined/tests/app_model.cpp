#include "combined/model/app.h"

#include "gtest/gtest.h"

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
