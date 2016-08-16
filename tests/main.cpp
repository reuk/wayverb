#include "gtest/gtest.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::GTEST_FLAG(filter) = "*mesh*:*boundary*";
    ::testing::GTEST_FLAG(filter) = "*reflector*";
    return RUN_ALL_TESTS();
}
