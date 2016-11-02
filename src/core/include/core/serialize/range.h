#pragma once

#include "common/serialize/vec.h"

namespace core {
template <typename t>
template <typename archive>
void util::range<t>::serialize(archive& a) {
    a(cereal::make_nvp("min", min), cereal::make_nvp("max", max));
}
}//namespace core 
