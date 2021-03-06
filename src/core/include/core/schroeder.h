#pragma once

#include "core/linear_regression.h"

#include "utilities/aligned/vector.h"
#include "utilities/decibels.h"
#include "utilities/map_to_vector.h"
#include "utilities/string_builder.h"

#include "glm/glm.hpp"

#include <cmath>

namespace wayverb {
namespace core {

template <typename It>
auto squared_integrated(It begin, It end) {
    using numeric = std::decay_t<decltype(*begin)>;
    util::aligned::vector<numeric> ret;
    numeric running_total{};
    for (; begin != end; ++begin) {
        using std::pow;
        running_total += *begin * *begin;
        ret.emplace_back(running_total);
    }
    return ret;
}

struct reverb_time final {
    /// Reverb times are returned in samples and must be corrected against an
    /// appropriate sampling rate to find the time in seconds.
    double samples;

    /// The product-moment correlation coefficient.
    /// Should ideally be between -1 <= r < -0.95, where lower is better.
    /// If -0.95 <= r the reverb time probably shouldn't be trusted.
    double r;
};

/// Uses backward-curve integration method.
template <typename It>
reverb_time decay_time_from_points(
        It begin, It end, float begin_db, float end_db, float min_db) {
    const auto integrated = squared_integrated(
            std::make_reverse_iterator(end), std::make_reverse_iterator(begin));

    size_t time{0};
    const auto in_db_with_times = util::map_to_vector(
            integrated.crbegin(), integrated.crend(), [&](auto i) {
                return glm::vec2{time++,
                                 util::decibels::p2db(i / integrated.back())};
            });

    const auto comparator = [](auto i, auto j) { return i.y < j; };
    const auto regression_end = std::lower_bound(in_db_with_times.crbegin(),
                                                 in_db_with_times.crend(),
                                                 begin_db,
                                                 comparator);
    const auto regression_begin = std::lower_bound(in_db_with_times.crbegin(),
                                                   in_db_with_times.crend(),
                                                   end_db,
                                                   comparator);

    const auto regression =
            simple_linear_regression(regression_begin, regression_end);

    //  Find time between desired points on regression line.
    const auto find_time = [&](auto level) {
        return (level - regression.c) / regression.m;
    };

    return {find_time(min_db), regression.r};
}

template <typename It>
reverb_time rt20(It begin, It end) {
    return decay_time_from_points(begin, end, -5, -25, -60);
}

template <typename It>
reverb_time rt30(It begin, It end) {
    return decay_time_from_points(begin, end, -5, -35, -60);
}

template <typename It>
reverb_time edt(It begin, It end) {
    return decay_time_from_points(begin, end, 0, -10, -60);
}

}  // namespace core
}  // namespace wayverb
