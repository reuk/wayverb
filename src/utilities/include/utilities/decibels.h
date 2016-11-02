#pragma once

#include <cmath>

namespace util {
namespace decibels {

template <typename T>
inline T p2db(T p) {
    using std::pow;
    return 10 * log10(p);
}

template <typename T>
inline T db2p(T db) {
    using std::pow;
    return pow(10, db / 10);
}

template <typename T>
inline T a2db(T a) {
    using std::log10;
    return 20 * log10(a);
}

template <typename T>
inline T db2a(T db) {
    using std::pow;
    return pow(10, db / 20);
}

}  // namespace decibels
}  // namespace util
