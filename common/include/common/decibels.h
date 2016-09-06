#pragma once

#include <cmath>

namespace decibels {

inline double p2db(double p) {
    return 10 * std::log10(p);
}
inline double db2p(double db) {
    return std::pow(10, db / 10);
}

inline double a2db(double a) {
    return 20 * std::log10(a);
}
inline double db2a(double db) {
    return std::pow(10, db / 20);
}

}  // namespace decibels