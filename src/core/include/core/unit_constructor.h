#pragma once

#include "core/cl/scene_structs.h"

namespace wayverb {
namespace core {

template <typename T>
struct unit_constructor;

template <>
struct unit_constructor<float> final {
    static constexpr float value{1.0f};
};

template <>
struct unit_constructor<cl_float1> final {
    static constexpr cl_float1 value{{1.0f}};
};

template <>
struct unit_constructor<cl_float2> final {
    static constexpr cl_float2 value{{1.0f, 1.0f}};
};

template <>
struct unit_constructor<cl_float4> final {
    static constexpr cl_float4 value{{1.0f, 1.0f, 1.0f, 1.0f}};
};

template <>
struct unit_constructor<cl_float8> final {
    static constexpr cl_float8 value{
            {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}};
};

template <typename T>
constexpr auto unit_constructor_v = unit_constructor<T>::value;

}  // namespace core
}  // namespace wayverb
