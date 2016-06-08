#include "common/extended_algorithms.h"

#include "gtest/gtest.h"

#include <array>

template <typename T>
struct PrintType;

TEST(zip, zip) {
    std::vector<double> v0{1, 2, 3, 4, 5};
    std::vector<int> v1{4, 5, 6, 6, 7};
    std::array<char, 6> v2{{'a', 'b', 'c', 'd', 'e', 'f'}};
    auto zipped = proc::zip(v0, v1, v2);
    std::vector<std::tuple<double, int, char>> predicted{
            std::make_tuple(1, 4, 'a'),
            std::make_tuple(2, 5, 'b'),
            std::make_tuple(3, 6, 'c'),
            std::make_tuple(4, 6, 'd'),
            std::make_tuple(5, 7, 'e')};
    ASSERT_EQ(zipped, predicted);
}
