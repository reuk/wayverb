#pragma once

#include "utilities/aligned/allocator.h"

#include <map>

namespace aligned {

template <typename T, typename U, typename Cmp = std::less<T>>
using map = std::map<T, U, Cmp, allocator<std::pair<const T, U>>>;

}  // namespace aligned
