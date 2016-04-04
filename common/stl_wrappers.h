#pragma once

#include <algorithm>
#include <numeric>

namespace proc {

#define STL_WRAPPER(name)                                               \
    template <typename Coll, typename... Args>                          \
    auto name(const Coll& c, Args&&... args) {                          \
        return std::name(                                               \
            std::cbegin(c), std::cend(c), std::forward<Args>(args)...); \
    }                                                                   \
    template <typename Coll, typename... Args>                          \
    auto name(Coll& c, Args&&... args) {                                \
        return std::name(                                               \
            std::begin(c), std::end(c), std::forward<Args>(args)...);   \
    }

STL_WRAPPER(all_of)
STL_WRAPPER(any_of)
STL_WRAPPER(none_of)
STL_WRAPPER(for_each)
STL_WRAPPER(find)
STL_WRAPPER(find_if)
STL_WRAPPER(find_if_not)
STL_WRAPPER(find_end)
STL_WRAPPER(find_first_of)
STL_WRAPPER(adjacent_find)
STL_WRAPPER(count)
STL_WRAPPER(count_if)
STL_WRAPPER(mismatch)
STL_WRAPPER(equal)
STL_WRAPPER(is_permutation)
STL_WRAPPER(search)
STL_WRAPPER(search_n)

STL_WRAPPER(copy)
STL_WRAPPER(copy_n)
STL_WRAPPER(copy_if)
STL_WRAPPER(copy_backward)
STL_WRAPPER(move)
STL_WRAPPER(move_backward)
STL_WRAPPER(swap_ranges)
STL_WRAPPER(transform)
STL_WRAPPER(replace)
STL_WRAPPER(replace_if)
STL_WRAPPER(replace_copy)
STL_WRAPPER(replace_copy_if)
STL_WRAPPER(fill)
STL_WRAPPER(fill_n)
STL_WRAPPER(generate)
STL_WRAPPER(generate_n)
STL_WRAPPER(remove)
STL_WRAPPER(remove_if)
STL_WRAPPER(remove_copy)
STL_WRAPPER(remove_copy_if)
STL_WRAPPER(unique)
STL_WRAPPER(unique_copy)
STL_WRAPPER(reverse)
STL_WRAPPER(reverse_copy)
STL_WRAPPER(rotate)
STL_WRAPPER(rotate_copy)
STL_WRAPPER(random_shuffle)
STL_WRAPPER(shuffle)

STL_WRAPPER(is_partitioned)
STL_WRAPPER(partition)
STL_WRAPPER(stable_partition)
STL_WRAPPER(partition_copy)
STL_WRAPPER(partition_point)

STL_WRAPPER(sort)
STL_WRAPPER(stable_sort)
STL_WRAPPER(partial_sort)
STL_WRAPPER(partial_sort_copy)
STL_WRAPPER(is_sorted)
STL_WRAPPER(is_sorted_until)
STL_WRAPPER(nth_element)

STL_WRAPPER(lower_bound)
STL_WRAPPER(upper_bound)
STL_WRAPPER(equal_range)
STL_WRAPPER(binary_search)

STL_WRAPPER(merge)
STL_WRAPPER(inplace_merge)
STL_WRAPPER(includes)
STL_WRAPPER(set_union)
STL_WRAPPER(set_intersection)
STL_WRAPPER(set_difference)
STL_WRAPPER(set_symmetric_difference)

STL_WRAPPER(make_heap)
STL_WRAPPER(sort_heap)
STL_WRAPPER(is_heap)
STL_WRAPPER(is_heap_until)

STL_WRAPPER(min_element)
STL_WRAPPER(max_element)

STL_WRAPPER(lexicographical_compare)
STL_WRAPPER(next_permutation)
STL_WRAPPER(prev_permutation)

STL_WRAPPER(accumulate)
STL_WRAPPER(adjacent_difference)
STL_WRAPPER(inner_product)
STL_WRAPPER(partial_sum)
STL_WRAPPER(iota)
}
