#pragma once

namespace util {

constexpr int popcount(size_t t) {
    int ret = 0;
    for (; t; t &= t - 1) {
        ++ret;
    }
    return ret;
}

}  // namespace util
