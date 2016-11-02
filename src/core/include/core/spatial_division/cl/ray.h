#pragma once

//  please only include in .cpp files

#include <string>

namespace wayverb {
namespace core {
namespace cl_sources {
const std::string ray(R"(

#ifndef RAY_HEADER__
#define RAY_HEADER__

typedef struct {
    float3 position;
    float3 direction;
} Ray;

#endif

)");
}  // namespace cl_sources
}  // namespace core
}  // namespace wayverb
