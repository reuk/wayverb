#pragma once

#include "common/aligned/allocator.h"

#include <set>

namespace aligned {

template <typename T, typename Cmp = std::less<T>>
using set = std::set<T, Cmp, allocator<T>>;

}  // namespace aligned

