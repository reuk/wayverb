#pragma once

#include <cmath>

inline double a2db(double a) {
    return 20 * std::log10(a);
}
inline double db2a(double db) {
    return std::pow(10, db / 20);
}
