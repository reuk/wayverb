#pragma once

#include "common/aligned/vector.h"

#include "sndfile.hh"

#include <string>
#include <vector>

namespace snd {
void write(const std::string& fname,
           const aligned::vector<aligned::vector<float>>& signal,
           double sample_rate,
           size_t bit_depth);
}  // namespace snd
