#pragma once

#include "common/cl/scene_structs.h"

template <typename T>
struct unit_constructor;

template<> struct unit_constructor<float> final {
    static constexpr float value{1.0f};
};

template<> struct unit_constructor<double> final {
    static constexpr double value{1.0};
};

template<> struct unit_constructor<volume_type> final {
    static constexpr volume_type value{
            {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}};
};

template <typename T>
constexpr auto unit_constructor_v{unit_constructor<T>::value};
