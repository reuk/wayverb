#include "common/recursive_vector.h"

#include "gtest/gtest.h"

#include <random>

TEST(recursive_vector_impl, size_capacity) {
    {
        detail::recursive_vector_impl<int> vec{};
        ASSERT_EQ(vec.size(), 0);
        ASSERT_EQ(vec.capacity(), 0);
    }

    {
        detail::recursive_vector_impl<int> vec{100};
        ASSERT_EQ(vec.size(), 0);
        ASSERT_EQ(vec.capacity(), 100);
    }
}

TEST(recursive_vector_impl, swap) {
    detail::recursive_vector_impl<int> a{0};
    detail::recursive_vector_impl<int> b{100};

    ASSERT_EQ(a.size(), 0);
    ASSERT_EQ(b.size(), 0);

    ASSERT_EQ(a.capacity(), 0);
    ASSERT_EQ(b.capacity(), 100);

    a.swap(b);

    ASSERT_EQ(b.size(), 0);
    ASSERT_EQ(a.size(), 0);

    ASSERT_EQ(b.capacity(), 0);
    ASSERT_EQ(a.capacity(), 100);
}

class non_trivial final {
public:
    non_trivial(const std::string& str, double value)
            : str_{str}
            , value_{value} {}

    std::string get_str() const { return str_; }
    double get_value() const { return value_; }

private:
    std::string str_;
    double value_;

    friend auto to_tuple(const non_trivial& a) {
        return std::tie(a.str_, a.value_);
    }
};

bool operator==(const non_trivial& a, const non_trivial& b) {
    return to_tuple(a) == to_tuple(b);
}

bool operator!=(const non_trivial& a, const non_trivial& b) {
    return !(a == b);
}

TEST(recursive_vector_impl, construct) {
    detail::recursive_vector_impl<non_trivial> vec{100};
    ASSERT_EQ(vec.capacity(), 100);

    vec.construct("hello", 1);
    ASSERT_EQ(vec.size(), 1);

    vec.construct("world", 2);
    ASSERT_EQ(vec.size(), 2);

    ASSERT_EQ(*(vec.begin() + 0), (non_trivial{"hello", 1.0}));
    ASSERT_EQ(*(vec.begin() + 1), (non_trivial{"world", 2.0}));
}

TEST(recursive_vector_impl, copy) {
    detail::recursive_vector_impl<non_trivial> a{100};
    a.construct("hello", 1);
    a.construct("world", 2);

    const auto b{a};
    ASSERT_EQ(b.size(), a.size());

    ASSERT_EQ(*(a.begin() + 0), (non_trivial{"hello", 1.0}));
    ASSERT_EQ(*(a.begin() + 1), (non_trivial{"world", 2.0}));
    ASSERT_EQ(*(b.begin() + 0), (non_trivial{"hello", 1.0}));
    ASSERT_EQ(*(b.begin() + 1), (non_trivial{"world", 2.0}));
}

TEST(recursive_vector_impl, move) {
    detail::recursive_vector_impl<non_trivial> a{100};
    a.construct("hello", 1);
    a.construct("world", 2);

    const auto b{std::move(a)};

    ASSERT_EQ(a.size(), 0);
    ASSERT_EQ(a.capacity(), 0);

    ASSERT_EQ(*(b.begin() + 0), (non_trivial{"hello", 1.0}));
    ASSERT_EQ(*(b.begin() + 1), (non_trivial{"world", 2.0}));
}

TEST(recursive_vector_impl, copy_assign) {
    detail::recursive_vector_impl<non_trivial> a{100};
    a.construct("hello", 1);
    a.construct("world", 2);

    detail::recursive_vector_impl<non_trivial> b{};
    b = a;
    ASSERT_EQ(b.size(), a.size());

    ASSERT_EQ(*(a.begin() + 0), (non_trivial{"hello", 1.0}));
    ASSERT_EQ(*(a.begin() + 1), (non_trivial{"world", 2.0}));
    ASSERT_EQ(*(b.begin() + 0), (non_trivial{"hello", 1.0}));
    ASSERT_EQ(*(b.begin() + 1), (non_trivial{"world", 2.0}));
}

TEST(recursive_vector_impl, move_assign) {
    detail::recursive_vector_impl<non_trivial> a{100};
    a.construct("hello", 1);
    a.construct("world", 2);

    detail::recursive_vector_impl<non_trivial> b{};
    b = std::move(a);

    ASSERT_EQ(a.size(), 0);
    ASSERT_EQ(a.capacity(), 0);

    ASSERT_EQ(*(b.begin() + 0), (non_trivial{"hello", 1.0}));
    ASSERT_EQ(*(b.begin() + 1), (non_trivial{"world", 2.0}));
}

////////////////////////////////////////////////////////////////////////////////

TEST(recursive_vector, size_capacity) {
    const recursive_vector<int> vec{}; 
    ASSERT_EQ(vec.size(), 0);
    ASSERT_EQ(vec.capacity(), 0);
}

TEST(recursive_vector, insert) {
    recursive_vector<int> vec{};
    const auto a = {1, 2, 3, 4};
    const auto i{vec.insert(vec.end(), a.begin(), a.end())};

    ASSERT_EQ(std::distance(vec.begin(), i), 0);
    ASSERT_EQ(vec.size(), a.size());
    ASSERT_GE(vec.capacity(), a.size());

    ASSERT_EQ(*(vec.begin() + 0), 1);
    ASSERT_EQ(*(vec.begin() + 1), 2);
    ASSERT_EQ(*(vec.begin() + 2), 3);
    ASSERT_EQ(*(vec.begin() + 3), 4);

    const auto b = {9, 8, 7, 6};
    const auto j{vec.insert(vec.begin(), b.begin(), b.end())};

    ASSERT_EQ(std::distance(vec.begin(), j), 0);
    ASSERT_EQ(vec.size(), a.size() + b.size());
    ASSERT_GE(vec.capacity(), a.size() + b.size());

    ASSERT_EQ(*(vec.begin() + 0), 9);
    ASSERT_EQ(*(vec.begin() + 1), 8);
    ASSERT_EQ(*(vec.begin() + 2), 7);
    ASSERT_EQ(*(vec.begin() + 3), 6);
    ASSERT_EQ(*(vec.begin() + 4), 1);
    ASSERT_EQ(*(vec.begin() + 5), 2);
    ASSERT_EQ(*(vec.begin() + 6), 3);
    ASSERT_EQ(*(vec.begin() + 7), 4);

    const auto x{10};
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 9);  ASSERT_EQ(std::distance(vec.begin(), k), 0); }
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 10); ASSERT_EQ(std::distance(vec.begin(), k), 0); }
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 11); ASSERT_EQ(std::distance(vec.begin(), k), 0); }
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 12); ASSERT_EQ(std::distance(vec.begin(), k), 0); }
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 13); ASSERT_EQ(std::distance(vec.begin(), k), 0); }
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 14); ASSERT_EQ(std::distance(vec.begin(), k), 0); }
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 15); ASSERT_EQ(std::distance(vec.begin(), k), 0); }
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 16); ASSERT_EQ(std::distance(vec.begin(), k), 0); }
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 17); ASSERT_EQ(std::distance(vec.begin(), k), 0); }
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 18); ASSERT_EQ(std::distance(vec.begin(), k), 0); }
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 19); ASSERT_EQ(std::distance(vec.begin(), k), 0); }
    { const auto k{vec.insert(vec.begin(), &x + 0, &x + 1)}; ASSERT_EQ(vec.size(), 20); ASSERT_EQ(std::distance(vec.begin(), k), 0); }

    ASSERT_EQ(*(vec.begin() + 0), 10);
    ASSERT_EQ(*(vec.begin() + 1), 10);
    ASSERT_EQ(*(vec.begin() + 2), 10);
    ASSERT_EQ(*(vec.begin() + 3), 10);
    ASSERT_EQ(*(vec.begin() + 4), 10);
    ASSERT_EQ(*(vec.begin() + 5), 10);
    ASSERT_EQ(*(vec.begin() + 6), 10);
    ASSERT_EQ(*(vec.begin() + 7), 10);
    ASSERT_EQ(*(vec.begin() + 8), 10);
    ASSERT_EQ(*(vec.begin() + 9), 10);
    ASSERT_EQ(*(vec.begin() + 10), 10);
    ASSERT_EQ(*(vec.begin() + 11), 10);
    ASSERT_EQ(*(vec.begin() + 12), 9);
    ASSERT_EQ(*(vec.begin() + 13), 8);
    ASSERT_EQ(*(vec.begin() + 14), 7);
    ASSERT_EQ(*(vec.begin() + 15), 6);
    ASSERT_EQ(*(vec.begin() + 16), 1);
    ASSERT_EQ(*(vec.begin() + 17), 2);
    ASSERT_EQ(*(vec.begin() + 18), 3);
    ASSERT_EQ(*(vec.begin() + 19), 4);

    const auto c = {11, 12};
    const auto k{vec.insert(vec.begin() + 10, c.begin(), c.end())};

    ASSERT_EQ(std::distance(vec.begin(), k), 10);
    ASSERT_EQ(vec.size(), 22);

    ASSERT_EQ(*(vec.begin() + 0), 10);
    ASSERT_EQ(*(vec.begin() + 1), 10);
    ASSERT_EQ(*(vec.begin() + 2), 10);
    ASSERT_EQ(*(vec.begin() + 3), 10);
    ASSERT_EQ(*(vec.begin() + 4), 10);
    ASSERT_EQ(*(vec.begin() + 5), 10);
    ASSERT_EQ(*(vec.begin() + 6), 10);
    ASSERT_EQ(*(vec.begin() + 7), 10);
    ASSERT_EQ(*(vec.begin() + 8), 10);
    ASSERT_EQ(*(vec.begin() + 9), 10);
    ASSERT_EQ(*(vec.begin() + 10), 11);
    ASSERT_EQ(*(vec.begin() + 11), 12);
    ASSERT_EQ(*(vec.begin() + 12), 10);
    ASSERT_EQ(*(vec.begin() + 13), 10);
    ASSERT_EQ(*(vec.begin() + 14), 9);
    ASSERT_EQ(*(vec.begin() + 15), 8);
    ASSERT_EQ(*(vec.begin() + 16), 7);
    ASSERT_EQ(*(vec.begin() + 17), 6);
    ASSERT_EQ(*(vec.begin() + 18), 1);
    ASSERT_EQ(*(vec.begin() + 19), 2);
    ASSERT_EQ(*(vec.begin() + 20), 3);
    ASSERT_EQ(*(vec.begin() + 21), 4);
}

TEST(recursive_vector, erase) {
    recursive_vector<int> vec{};
    const auto a = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    vec.insert(vec.begin(), a.begin(), a.end());

    vec.erase(vec.begin(), vec.begin() + 1);
    ASSERT_EQ(vec.size(), 9);

    ASSERT_EQ(*(vec.begin() + 0), 2);
    ASSERT_EQ(*(vec.begin() + 1), 3);
    ASSERT_EQ(*(vec.begin() + 2), 4);
    ASSERT_EQ(*(vec.begin() + 3), 5);
    ASSERT_EQ(*(vec.begin() + 4), 6);
    ASSERT_EQ(*(vec.begin() + 5), 7);
    ASSERT_EQ(*(vec.begin() + 6), 8);
    ASSERT_EQ(*(vec.begin() + 7), 9);
    ASSERT_EQ(*(vec.begin() + 8), 10);

    vec.erase(vec.begin() + 2, vec.begin() + 3);
    ASSERT_EQ(vec.size(), 8);

    ASSERT_EQ(*(vec.begin() + 0), 2);
    ASSERT_EQ(*(vec.begin() + 1), 3);
    ASSERT_EQ(*(vec.begin() + 2), 5);
    ASSERT_EQ(*(vec.begin() + 3), 6);
    ASSERT_EQ(*(vec.begin() + 4), 7);
    ASSERT_EQ(*(vec.begin() + 5), 8);
    ASSERT_EQ(*(vec.begin() + 6), 9);
    ASSERT_EQ(*(vec.begin() + 7), 10);

    vec.erase(vec.begin(), vec.end() - 1);
    ASSERT_EQ(vec.size(), 1);
    ASSERT_EQ(*(vec.begin() + 0), 10);

    vec.erase(vec.begin(), vec.end());
    ASSERT_EQ(vec.size(), 0);
}

TEST(recursive_vector, reserve) {
    recursive_vector<int> vec{};
    vec.reserve(100);

    ASSERT_EQ(vec.size(), 0);
    ASSERT_EQ(vec.capacity(), 100);
}

////////////////////////////////////////////////////////////////////////////////

TEST(recursive_vector_backed_set, insert) {
    recursive_vector_backed_set<int> set;

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

TEST(recursive_vector_backed_set, find) {
    recursive_vector_backed_set<int> set;

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
