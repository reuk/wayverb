#pragma once

#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

/// Enum denoting available filter types.
enum FilterType
{   FILTER_TYPE_WINDOWED_SINC
,   FILTER_TYPE_BIQUAD_ONEPASS
,   FILTER_TYPE_BIQUAD_TWOPASS
,   FILTER_TYPE_LINKWITZ_RILEY
};

/// Given a filter type and a vector of vector of float, return the
/// parallel-filtered and summed data, using the specified filtering method.
void filter
(   FilterType ft
,   std::vector <std::vector <std::vector <float>>> & data
,   float sr
,   float lo_cutoff
);
