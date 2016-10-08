#pragma once

#include "utilities/range.h"

namespace hrtf_data {

constexpr auto bands{8};
constexpr range<double> audible_range{20, 20000};

}//namespace hrtf_data
