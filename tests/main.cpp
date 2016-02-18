#include "logger.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
    Logger::restart();
    LOG_SCOPE;

    //    ::testing::GTEST_FLAG(filter) = "*compare_filters*";
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
