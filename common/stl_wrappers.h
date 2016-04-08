#pragma once

#include <algorithm>
#include <numeric>

namespace proc {

#define BOTH_STL_WRAPPER(name)                                        \
    template <typename Coll, typename... Args>                        \
    auto name(Coll&& c, Args&&... args) {                             \
        return std::name(                                             \
            std::begin(c), std::end(c), std::forward<Args>(args)...); \
    }

BOTH_STL_WRAPPER(all_of)
BOTH_STL_WRAPPER(any_of)
BOTH_STL_WRAPPER(none_of)
BOTH_STL_WRAPPER(for_each)
BOTH_STL_WRAPPER(find)
BOTH_STL_WRAPPER(find_if)
BOTH_STL_WRAPPER(find_if_not)
BOTH_STL_WRAPPER(find_end)
BOTH_STL_WRAPPER(find_first_of)
BOTH_STL_WRAPPER(adjacent_find)
BOTH_STL_WRAPPER(count)
BOTH_STL_WRAPPER(count_if)
BOTH_STL_WRAPPER(mismatch)
BOTH_STL_WRAPPER(equal)
BOTH_STL_WRAPPER(is_permutation)
BOTH_STL_WRAPPER(search)
BOTH_STL_WRAPPER(search_n)

BOTH_STL_WRAPPER(copy)
BOTH_STL_WRAPPER(copy_n)
BOTH_STL_WRAPPER(copy_if)
BOTH_STL_WRAPPER(copy_backward)
BOTH_STL_WRAPPER(move)
BOTH_STL_WRAPPER(move_backward)
BOTH_STL_WRAPPER(swap_ranges)
BOTH_STL_WRAPPER(transform)
BOTH_STL_WRAPPER(replace)
BOTH_STL_WRAPPER(replace_if)
BOTH_STL_WRAPPER(replace_copy)
BOTH_STL_WRAPPER(replace_copy_if)
BOTH_STL_WRAPPER(fill)
BOTH_STL_WRAPPER(fill_n)
BOTH_STL_WRAPPER(generate)
BOTH_STL_WRAPPER(generate_n)
BOTH_STL_WRAPPER(remove)
BOTH_STL_WRAPPER(remove_if)
BOTH_STL_WRAPPER(remove_copy)
BOTH_STL_WRAPPER(remove_copy_if)
BOTH_STL_WRAPPER(unique)
BOTH_STL_WRAPPER(unique_copy)
BOTH_STL_WRAPPER(reverse)
BOTH_STL_WRAPPER(reverse_copy)
BOTH_STL_WRAPPER(rotate)
BOTH_STL_WRAPPER(rotate_copy)
BOTH_STL_WRAPPER(random_shuffle)
BOTH_STL_WRAPPER(shuffle)

BOTH_STL_WRAPPER(is_partitioned)
BOTH_STL_WRAPPER(partition)
BOTH_STL_WRAPPER(stable_partition)
BOTH_STL_WRAPPER(partition_copy)
BOTH_STL_WRAPPER(partition_point)

BOTH_STL_WRAPPER(sort)
BOTH_STL_WRAPPER(stable_sort)
BOTH_STL_WRAPPER(partial_sort)
BOTH_STL_WRAPPER(partial_sort_copy)
BOTH_STL_WRAPPER(is_sorted)
BOTH_STL_WRAPPER(is_sorted_until)
BOTH_STL_WRAPPER(nth_element)

BOTH_STL_WRAPPER(lower_bound)
BOTH_STL_WRAPPER(upper_bound)
BOTH_STL_WRAPPER(equal_range)
BOTH_STL_WRAPPER(binary_search)

BOTH_STL_WRAPPER(merge)
BOTH_STL_WRAPPER(inplace_merge)
BOTH_STL_WRAPPER(includes)
BOTH_STL_WRAPPER(set_union)
BOTH_STL_WRAPPER(set_intersection)
BOTH_STL_WRAPPER(set_difference)
BOTH_STL_WRAPPER(set_symmetric_difference)

BOTH_STL_WRAPPER(make_heap)
BOTH_STL_WRAPPER(sort_heap)
BOTH_STL_WRAPPER(is_heap)
BOTH_STL_WRAPPER(is_heap_until)

BOTH_STL_WRAPPER(min_element)
BOTH_STL_WRAPPER(max_element)

BOTH_STL_WRAPPER(lexicographical_compare)
BOTH_STL_WRAPPER(next_permutation)
BOTH_STL_WRAPPER(prev_permutation)

BOTH_STL_WRAPPER(accumulate)
BOTH_STL_WRAPPER(adjacent_difference)
BOTH_STL_WRAPPER(inner_product)
BOTH_STL_WRAPPER(partial_sum)
BOTH_STL_WRAPPER(iota)
}  // namespace proc
