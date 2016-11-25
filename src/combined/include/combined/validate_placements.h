#pragma once

#include <algorithm>

/// \file validate_placements.h
/// Function(s) to check that all sources and receivers in a scene are placed
/// appropriately.

namespace wayverb {
namespace combined {

/// Given two ranges of positions, check that no item in the first range is too
/// close to any item in the second range.
/// WARNING this has O(a*b) complexity so use sparingly, on known-to-be-small
/// datasets.
template <typename ItA, typename ItB>
bool is_pairwise_distance_acceptable(
        ItA b_a, ItA e_a, ItB b_b, ItB e_b, double min_spacing) {
    for (auto a = b_a; a != e_a; ++a) {
        for (auto b = b_b; b != e_b; ++b) {
            if (distance(*a, *b) <= min_spacing) {
                return false;
            }
        }
    }
    return true;
}

template <typename It, typename Voxelised>
bool are_all_inside(It b, It e, const Voxelised& voxelised) {
    return std::all_of(
            b, e, [&](const auto& pos) { return inside(voxelised, pos); });
}

}  // namespace combined
}  // namespace wayverb
