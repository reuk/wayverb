#pragma once

#include "logger.h"

#include "cl.hpp"

std::ostream& operator<<(std::ostream& os, const cl_float3& f);

template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, const std::pair<T, U>& p) {
    Bracketer bracketer(os);
    return to_stream(os, p.first, "  ", p.second, "  ");
}

template <typename T, unsigned long U>
std::ostream& operator<<(std::ostream& os, const std::array<T, U>& arr) {
    Bracketer bracketer(os);
    for (const auto& i : arr)
        to_stream(os, i, "  ");
    return os;
}
