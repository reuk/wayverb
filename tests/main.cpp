#include "logger.h"
#include "gtest/gtest.h"

int main(int argc, char * argv[]) {
    Logger::restart();
    LOG_SCOPE;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
