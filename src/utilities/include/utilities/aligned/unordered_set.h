#pragma once

#include "utilities/aligned/allocator.h"

#include <unordered_set>

namespace util {
namespace aligned {

template <typename T,
          typename Hash = std::hash<T>,
          typename Equal = std::equal_to<T>>
using unordered_set = std::unordered_set<T, Hash, Equal, allocator<T>>;

}  // namespace aligned
}  // namespace util
