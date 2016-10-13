#pragma once

#include "common/cl/scene_structs.h"

constexpr const auto& get_absorption(const surface& t) { return t.absorption; }
constexpr const auto& get_absorption(const double& t) { return t; }
constexpr const auto& get_absorption(const float& t) { return t; }

template <typename T>
using absorption_t = std::decay_t<decltype(get_absorption(std::declval<T>()))>;
