#pragma once

#include "common/aligned/vector.h"

#include <cmath>

template <typename It>
auto squared_integrated(It begin, It end) {
    using numeric = std::decay_t<decltype(*begin)>;
    aligned::vector<numeric> ret;
    numeric running_total{};
    for (; begin != end; ++begin) {
        using std::pow;
        running_total += *begin * *begin;
        ret.push_back(running_total);
    }
    return ret;
}

/// uses backward-curve integration method
float decay_time_from_points(const aligned::vector<float>& sig,
                             float begin_db,
                             float end_db);

float rt20(const aligned::vector<float>& sig);
float rt30(const aligned::vector<float>& sig);
float edt(const aligned::vector<float>& sig);
