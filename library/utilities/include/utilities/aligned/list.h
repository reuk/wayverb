#pragma once

#include "utilities/aligned/allocator.h"

#include <list>

namespace aligned {

template <typename T>
using list = std::list<T, allocator<T>>;

}  // namespace aligned

