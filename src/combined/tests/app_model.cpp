#include "combined/model/app.h"

#include "gtest/gtest.h"

TEST(app_model, copy_assignment) {
    bool called = false;

    wayverb::combined::model::microphone a;

    a.connect_on_change([&](auto&) { called = true; });

    wayverb::combined::model::microphone b;
    b.set_shape(1);
    b.set_orientation(M_PI, 0.1);

    a = b;

    ASSERT_TRUE(called);
}

TEST(app_model, app_model) {

}
