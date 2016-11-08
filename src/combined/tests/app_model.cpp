#include "combined/model/app.h"

#include "gtest/gtest.h"

TEST(app_model, assignment) {
    bool called = false;

    wayverb::combined::model::microphone mic;

    mic.connect_on_change([&](auto&) { called = true; });

    mic = wayverb::combined::model::microphone{};

    ASSERT_TRUE(called);
}

TEST(app_model, app_model) {

}
