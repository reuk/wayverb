#pragma once

#include <algorithm>

namespace proc {

#define NON_MODIFYING(name)                                             \
    template <typename Coll, typename... Args>                          \
    auto name(const Coll& c, Args&&... args) {                          \
        return std::name(                                               \
            std::cbegin(c), std::cend(c), std::forward<Args>(args)...); \
    }

NON_MODIFYING(all_of)
NON_MODIFYING(any_of)
NON_MODIFYING(none_of)
NON_MODIFYING(for_each)
NON_MODIFYING(find)
NON_MODIFYING(find_if)
NON_MODIFYING(find_if_not)
NON_MODIFYING(find_end)
NON_MODIFYING(find_first_of)
NON_MODIFYING(adjacent_find)
NON_MODIFYING(count)
NON_MODIFYING(count_if)
NON_MODIFYING(mismatch)
NON_MODIFYING(equal)
NON_MODIFYING(is_permutation)
NON_MODIFYING(search)
NON_MODIFYING(search_n)

#define MODIFYING(name)                                               \
    template <typename Coll, typename... Args>                        \
    auto name(const Coll& c, Args&&... args) {                        \
        return std::name(                                             \
            std::begin(c), std::end(c), std::forward<Args>(args)...); \
    }

MODIFYING(copy)
MODIFYING(copy_n)
MODIFYING(copy_if)
MODIFYING(copy_backward)
MODIFYING(move)
MODIFYING(move_backward)
MODIFYING(swap_ranges)
MODIFYING(transform)
MODIFYING(replace)
MODIFYING(replace_if)
MODIFYING(replace_copy)
MODIFYING(replace_copy_if)
MODIFYING(fill)
MODIFYING(fill_n)
MODIFYING(generate)
MODIFYING(generate_n)
MODIFYING(remove)
MODIFYING(remove_if)
MODIFYING(remove_copy)
MODIFYING(remove_copy_if)
MODIFYING(unique)
MODIFYING(unique_copy)
MODIFYING(reverse)
MODIFYING(reverse_copy)
MODIFYING(rotate)
MODIFYING(rotate_copy)
MODIFYING(random_shuffle)
MODIFYING(shuffle)

NON_MODIFYING(is_partitioned)
MODIFYING(partition)
MODIFYING(stable_partition)
NON_MODIFYING(partition_copy)
NON_MODIFYING(partition_point)

MODIFYING(sort)
MODIFYING(stable_sort)
MODIFYING(partial_sort)
MODIFYING(partial_sort_copy)
NON_MODIFYING(is_sorted)
NON_MODIFYING(is_sorted_until)
NON_MODIFYING(nth_element)

NON_MODIFYING(lower_bound)
NON_MODIFYING(upper_bound)
NON_MODIFYING(equal_range)
NON_MODIFYING(binary_search)

NON_MODIFYING(merge)
MODIFYING(inplace_merge)
NON_MODIFYING(includes)
NON_MODIFYING(set_union)
NON_MODIFYING(set_intersection)
NON_MODIFYING(set_difference)
NON_MODIFYING(set_symmetric_difference)

MODIFYING(make_heap)
MODIFYING(sort_heap)
NON_MODIFYING(is_heap)
NON_MODIFYING(is_heap_until)

NON_MODIFYING(min_element)
NON_MODIFYING(max_element)

NON_MODIFYING(lexicographical_compare)
MODIFYING(next_permutation)
MODIFYING(prev_permutation)
}
