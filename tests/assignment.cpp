#include "gtest/gtest.h"

#include <map>
#include <iostream>

TEST(assignment, assignment) {
    struct alignas(16) troublesome {
        float s;
    };

    std::map<int, troublesome> m{{0, troublesome{}}};
    std::map<int, troublesome> n{{1, troublesome{}}};
    std::cout << "GOT HERE" << std::endl;
    m = std::move(n);
    std::cout << "PROBABLY WON'T GET HERE" << std::endl;
}
