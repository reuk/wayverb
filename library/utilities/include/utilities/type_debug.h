#pragma once

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

template <typename>
struct Type;

#define PRINT_TYPE(T) Type<T> CONCAT(t, __COUNTER__)
#define PRINT_TYPE_OF(T) PRINT_TYPE(decltype(T))
