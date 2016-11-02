#pragma once

#include "utilities/aligned/allocator.h"

#include <set>

namespace util {
namespace aligned {

template <typename T, typename Cmp = std::less<T>>
using set = std::set<T, Cmp, allocator<T>>;

}  // namespace aligned
}  // namespace util
