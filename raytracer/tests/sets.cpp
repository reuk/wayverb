#include "raytracer/image_source/tree.h"

#include "gtest/gtest.h"

#include <random>

template <template <class...> class Set>
void insert_test() {
    Set<int> set;

    std::default_random_engine engine{std::random_device{}()};
    std::uniform_int_distribution<int> distribution(-100, 100);

    for (auto i{0u}; i != 50; ++i) {
        set.insert(distribution(engine));
    }

    auto prev{set.begin()};
    for (auto i{std::next(prev)}; i != set.end(); ++i) {
        ASSERT_LT(*prev, *i);
        prev = i;
    }

    set.insert(0);

    {
        const auto pair{set.insert(0)};
        ASSERT_FALSE(pair.second);  //  value is already in set
    }

    {
        const auto pair{set.insert(200)};
        ASSERT_TRUE(pair.second);  //  value was not already in set
    }
}

template <template <class...> class Set>
void find_test() {
    Set<int> set;

    std::default_random_engine engine{std::random_device{}()};
    std::uniform_int_distribution<int> distribution(-100, 100);

    for (auto i{0u}; i != 50; ++i) {
        set.insert(distribution(engine));
    }

    for (auto i : {0, 200, 400}) {
        set.insert(i);
        const auto it{set.find(i)};
        ASSERT_NE(it, set.end());
        ASSERT_EQ(*it, i);
    }
}

//----------------------------------------------------------------------------//

TEST(vector_backed_set, insert) { insert_test<raytracer::vector_backed_set>(); }
TEST(vector_backed_set, find) { find_test<raytracer::vector_backed_set>(); }

//----------------------------------------------------------------------------//

TEST(recursive_set_adapter_vector, insert) {
    insert_test<raytracer::recursive_vector_set>();
}
TEST(recursive_set_adapter_vector, find) {
    find_test<raytracer::recursive_vector_set>();
}

//----------------------------------------------------------------------------//

TEST(recursive_set_adapter_set, insert) {
    insert_test<raytracer::recursive_set>();
}
TEST(recursive_set_adapter_set, find) { find_test<raytracer::recursive_set>(); }
