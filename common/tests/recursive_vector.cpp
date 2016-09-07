#include "common/recursive_vector.h"

#include "gtest/gtest.h"

TEST(recursive_vector, impl) {
    {
        recursive_vector<int> vec{};
    }
}

TEST(recursive_vector, recursive_vector_backed_set) {
    { 
        recursive_vector_backed_set<int> ints{}; 
        ints.insert(1);
    }
}
