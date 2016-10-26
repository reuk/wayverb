#pragma once

#include <tuple>
#include <algorithm>

namespace {
// std::ostream& operator<<(std::ostream& os, const index& i) {
//    return os << "[" << i.x << ", " << i.y << ", " << i.z << "]";
//}

template <typename T>
struct locator final {
    T x, y, z;
};

template <typename T>
constexpr bool operator==(const locator<T>& a, const locator<T>& b) {
    return std::make_tuple(a.x, a.y, a.z) == std::make_tuple(b.x, b.y, b.z);
}

template <typename T>
constexpr bool operator!=(const locator<T>& a, const locator<T>& b) {
    return !(a == b);
}

constexpr size_t triangle(size_t i) { return (i * (i + 1)) / 2; }

static_assert(triangle(0) == 0, "triangle");
static_assert(triangle(1) == 1, "triangle");
static_assert(triangle(2) == 3, "triangle");
static_assert(triangle(3) == 6, "triangle");
static_assert(triangle(4) == 10, "triangle");
static_assert(triangle(5) == 15, "triangle");

constexpr size_t tetrahedron(size_t i) { return (i * (i + 1) * (i + 2)) / 6; }

static_assert(tetrahedron(0) == 0, "tetrahdedron");
static_assert(tetrahedron(1) == 1, "tetrahdedron");
static_assert(tetrahedron(2) == 4, "tetrahdedron");
static_assert(tetrahedron(3) == 10, "tetrahdedron");
static_assert(tetrahedron(4) == 20, "tetrahdedron");
static_assert(tetrahedron(5) == 35, "tetrahdedron");

constexpr locator<size_t> to_locator(size_t ind) {
    auto x = 0u;
    for (auto i = 1u;; ++i) {
        auto tri = triangle(i);
        if (ind < tri) {
            x = i - 1;
            break;
        }
        ind -= tri;
    }

    auto y = 0u;
    for (auto i = 1u;; ++i) {
        if (ind < i) {
            y = i - 1;
            break;
        }
        ind -= i;
    }

    auto z = ind;

    return locator<size_t>{x, y, z};
}

static_assert(to_locator(0) == locator<size_t>{0, 0, 0}, "to_locator");

static_assert(to_locator(1) == locator<size_t>{1, 0, 0}, "to_locator");
static_assert(to_locator(2) == locator<size_t>{1, 1, 0}, "to_locator");
static_assert(to_locator(3) == locator<size_t>{1, 1, 1}, "to_locator");

static_assert(to_locator(4) == locator<size_t>{2, 0, 0}, "to_locator");
static_assert(to_locator(5) == locator<size_t>{2, 1, 0}, "to_locator");
static_assert(to_locator(6) == locator<size_t>{2, 1, 1}, "to_locator");
static_assert(to_locator(7) == locator<size_t>{2, 2, 0}, "to_locator");
static_assert(to_locator(8) == locator<size_t>{2, 2, 1}, "to_locator");
static_assert(to_locator(9) == locator<size_t>{2, 2, 2}, "to_locator");

static_assert(to_locator(10) == locator<size_t>{3, 0, 0}, "to_locator");
static_assert(to_locator(11) == locator<size_t>{3, 1, 0}, "to_locator");
static_assert(to_locator(12) == locator<size_t>{3, 1, 1}, "to_locator");
static_assert(to_locator(13) == locator<size_t>{3, 2, 0}, "to_locator");
static_assert(to_locator(14) == locator<size_t>{3, 2, 1}, "to_locator");
static_assert(to_locator(15) == locator<size_t>{3, 2, 2}, "to_locator");
static_assert(to_locator(16) == locator<size_t>{3, 3, 0}, "to_locator");
static_assert(to_locator(17) == locator<size_t>{3, 3, 1}, "to_locator");
static_assert(to_locator(18) == locator<size_t>{3, 3, 2}, "to_locator");
static_assert(to_locator(19) == locator<size_t>{3, 3, 3}, "to_locator");

constexpr size_t abs(int i) { return std::max(i, -i); }

template <typename T>
constexpr void swap(T& a, T& b) {
    T t = a;
    a   = b;
    b   = t;
}

constexpr locator<size_t> fold_locator(const locator<int>& i) {
    size_t x     = abs(i.x);
    size_t y     = abs(i.y);
    size_t z     = abs(i.z);
    size_t plane = x + 1;
    if (plane <= y) {
        swap(x, y);
    }
    if (plane <= z) {
        swap(x, z);
    }
    if (y < z) {
        swap(y, z);
    }
    return locator<size_t>{x, y, z};
}

static_assert(fold_locator(locator<int>{0, 0, 0}) == locator<size_t>{0, 0, 0},
              "fold_locator");

static_assert(fold_locator(locator<int>{1, 0, 0}) == locator<size_t>{1, 0, 0},
              "fold_locator");
static_assert(fold_locator(locator<int>{-1, 0, 0}) == locator<size_t>{1, 0, 0},
              "fold_locator");
static_assert(fold_locator(locator<int>{0, 1, 0}) == locator<size_t>{1, 0, 0},
              "fold_locator");
static_assert(fold_locator(locator<int>{0, -1, 0}) == locator<size_t>{1, 0, 0},
              "fold_locator");
static_assert(fold_locator(locator<int>{0, 0, 1}) == locator<size_t>{1, 0, 0},
              "fold_locator");
static_assert(fold_locator(locator<int>{0, 0, -1}) == locator<size_t>{1, 0, 0},
              "fold_locator");

static_assert(fold_locator(locator<int>{4, 0, 3}) == locator<size_t>{4, 3, 0},
              "fold_locator");
static_assert(fold_locator(locator<int>{-4, 0, 3}) == locator<size_t>{4, 3, 0},
              "fold_locator");

constexpr size_t to_index(const locator<size_t>& l) {
    return tetrahedron(l.x) + triangle(l.y) + l.z;
}

static_assert(0 == to_index(locator<size_t>{0, 0, 0}), "to_locator");

static_assert(1 == to_index(locator<size_t>{1, 0, 0}), "to_locator");
static_assert(2 == to_index(locator<size_t>{1, 1, 0}), "to_locator");
static_assert(3 == to_index(locator<size_t>{1, 1, 1}), "to_locator");

static_assert(4 == to_index(locator<size_t>{2, 0, 0}), "to_locator");
static_assert(5 == to_index(locator<size_t>{2, 1, 0}), "to_locator");
static_assert(6 == to_index(locator<size_t>{2, 1, 1}), "to_locator");
static_assert(7 == to_index(locator<size_t>{2, 2, 0}), "to_locator");
static_assert(8 == to_index(locator<size_t>{2, 2, 1}), "to_locator");
static_assert(9 == to_index(locator<size_t>{2, 2, 2}), "to_locator");

static_assert(10 == to_index(locator<size_t>{3, 0, 0}), "to_locator");
static_assert(11 == to_index(locator<size_t>{3, 1, 0}), "to_locator");
static_assert(12 == to_index(locator<size_t>{3, 1, 1}), "to_locator");
static_assert(13 == to_index(locator<size_t>{3, 2, 0}), "to_locator");
static_assert(14 == to_index(locator<size_t>{3, 2, 1}), "to_locator");
static_assert(15 == to_index(locator<size_t>{3, 2, 2}), "to_locator");
static_assert(16 == to_index(locator<size_t>{3, 3, 0}), "to_locator");
static_assert(17 == to_index(locator<size_t>{3, 3, 1}), "to_locator");
static_assert(18 == to_index(locator<size_t>{3, 3, 2}), "to_locator");
static_assert(19 == to_index(locator<size_t>{3, 3, 3}), "to_locator");

}  // namespace
