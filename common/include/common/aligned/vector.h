#pragma once

#include "common/aligned/allocator.h"

#include <vector>

namespace aligned {

template <typename T>
using vector = std::vector<T, allocator<T>>;

}  // namespace aligned
