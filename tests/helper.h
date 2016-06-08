#pragma once

#include "gtest/gtest.h"

#include <tuple>

template <typename T, typename U>
void vec_assert_eq(T t, U u) {
    t.zip(u).for_each(
            [](auto i) { ASSERT_EQ(std::get<0>(i), std::get<1>(i)); });
}
