#include "waveguide/cl/filter_structs.h"

const std::string cl_representation<memory_biquad>::value{
        R"(
#ifndef BIQUAD_MEMORY_DEFINITION__
#define BIQUAD_MEMORY_DEFINITION__
typedef struct {
    real array[)" + std::to_string(memory_biquad::order) + R"(];
} memory_)" + std::to_string(memory_biquad::order) + R"(;

typedef memory_)" + std::to_string(memory_biquad::order) + R"( memory_biquad;
#endif
)"};

const std::string cl_representation<coefficients_biquad>::value{
        R"(
#ifndef BIQUAD_COEFFICIENTS_DEFINITION__
#define BIQUAD_COEFFICIENTS_DEFINITION__
typedef struct {
    real b[)" + std::to_string(coefficients_biquad::order + 1) + R"(];
    real a[)" + std::to_string(coefficients_biquad::order + 1) + R"(];
} coefficients_)" + std::to_string(coefficients_biquad::order) + R"(;

typedef coefficients_)" + std::to_string(coefficients_biquad::order) + R"( coefficients_biquad;
#endif
)"};

const std::string cl_representation<memory_canonical>::value{
        R"(
#ifndef CANONICAL_MEMORY_DEFINITION__
#define CANONICAL_MEMORY_DEFINITION__
typedef struct {
    real array[)" + std::to_string(memory_canonical::order) + R"(];
} memory_)" + std::to_string(memory_canonical::order) + R"(;

typedef memory_)" + std::to_string(memory_canonical::order) + R"( memory_canonical;
#endif
)"};

const std::string cl_representation<coefficients_canonical>::value{
        R"(
#ifndef CANONICAL_COEFFICIENTS_DEFINITION__
#define CANONICAL_COEFFICIENTS_DEFINITION__
typedef struct {
    real b[)" + std::to_string(coefficients_canonical::order + 1) + R"(];
    real a[)" + std::to_string(coefficients_canonical::order + 1) + R"(];
} coefficients_)" + std::to_string(coefficients_canonical::order) + R"(;

typedef coefficients_)" + std::to_string(coefficients_canonical::order) + R"( coefficients_canonical;
#endif
)"};

