#include "glog/logging.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    //    ::testing::GTEST_FLAG(filter) = "*waveguide*";
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
