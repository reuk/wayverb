#include "common/decibels.h"
#include "common/linear_regression.h"
#include "common/map_to_vector.h"
#include "common/schroeder.h"
#include "common/stl_wrappers.h"

#include "glm/glm.hpp"

/// http://labrosa.ee.columbia.edu/~dpwe/papers/Schro65-reverb.pdf

float decay_time_from_points(const aligned::vector<float>& sig,
                             float begin_db,
                             float end_db) {
    const auto integrated{squared_integrated(sig.crbegin(), sig.crend())};

    size_t time{0};
    const auto in_db_with_times{map_to_vector(
            integrated.crbegin(), integrated.crend(), [&](auto i) {
                return glm::vec2{time++, decibels::p2db(i / integrated.back())};
            })};

    const auto comparator{[](auto i, auto j) { return i.y < j; }};
    const auto begin{std::lower_bound(in_db_with_times.crbegin(),
                                      in_db_with_times.crend(),
                                      begin_db,
                                      comparator)};
    const auto end{std::lower_bound(in_db_with_times.crbegin(),
                                    in_db_with_times.crend(),
                                    end_db,
                                    comparator)};

    const auto regression{simple_linear_regression(end, begin)};

    //  Find time between desired points on regression line.
    const auto find_time{
            [&](auto level) { return (level - regression.c) / regression.m; }};

    return find_time(end_db) - find_time(begin_db);
}

float rt20(const aligned::vector<float>& sig) {
    return decay_time_from_points(sig, -5, -25);
}

float rt30(const aligned::vector<float>& sig) {
    return decay_time_from_points(sig, -5, -35);
}

float edt(const aligned::vector<float>& sig) {
    return decay_time_from_points(sig, 0, -10);
}
