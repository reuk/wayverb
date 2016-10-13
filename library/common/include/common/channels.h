#pragma once

#include "common/absorption.h"
#include "common/cl/scene_structs.h"

template <typename T>
struct channels final {
    using type = ::detail::components_t<T>;
};

template <typename T>
using channels_t = typename channels<T>::type;

template <typename T>
constexpr auto channels_v{channels_t<T>{}};

template <>
struct channels<surface> final {
    using type = channels_t<absorption_t<surface>>;
};
