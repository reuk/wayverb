#pragma once

#include "common/aligned/allocator.h"

#include <unordered_map>

namespace aligned {

template <typename T,
          typename U,
          typename Hash  = std::hash<T>,
          typename Equal = std::equal_to<T>>
using unordered_map =
        std::unordered_map<T, U, Hash, Equal, allocator<std::pair<const T, U>>>;

}  // namespace aligned
