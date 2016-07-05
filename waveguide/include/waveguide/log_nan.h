#pragma once

#include "waveguide/rectangular_program.h"

template <typename Fun, typename T>
bool is_any(const std::vector<T>& t, const Fun& fun) {
    return proc::any_of(t, [&fun](const auto& i) { return is_any(i, fun); });
}

template <typename Fun, int I>
bool is_any(const RectangularProgram::BoundaryDataArray<I>& t, const Fun& fun) {
    return proc::any_of(t.array,
                        [&fun](const auto& i) { return is_any(i, fun); });
}

template <typename Fun>
bool is_any(const RectangularProgram::BoundaryData& t, const Fun& fun) {
    return proc::any_of(t.filter_memory.array,
                        [&fun](const auto& i) { return is_any(i, fun); });
}

template <typename Fun, typename T>
bool is_any(const T& t, const Fun& fun) {
    return fun(t);
}

template <typename Fun, typename T>
auto find_any(const T& t, const Fun& fun) {
    return proc::find_if(t, [&fun](const auto& i) { return is_any(i, fun); });
}

template <typename Fun, typename T>
auto log_find_any(const T& t,
                  const std::string& identifier,
                  const std::string& func,
                  const Fun& fun) {
    auto it = find_any(t, fun);
    if (it != std::end(t)) {
        std::cerr << identifier << " " << func
                  << " index: " << it - std::begin(t) << ", value: " << *it
                  << std::endl;
    }
    return it;
}

template <typename T>
auto log_nan(const T& t, const std::string& identifier) {
    return log_find_any(
            t, identifier, "nan", [](auto i) { return std::isnan(i); });
}

template <typename T>
auto log_inf(const T& t, const std::string& identifier) {
    return log_find_any(
            t, identifier, "inf", [](auto i) { return std::isinf(i); });
}

template <typename T>
auto log_nonzero(const T& t, const std::string& identifier) {
    return log_find_any(t, identifier, "nonzero", [](auto i) { return i; });
}

template <typename T>
bool log_nan_or_nonzero_or_inf(const T& t, const std::string& identifier) {
    if (log_nan(t, identifier) != std::end(t)) {
        return true;
    }
    if (log_inf(t, identifier) != std::end(t)) {
        return true;
    }
    log_nonzero(t, identifier);
    return false;
}
