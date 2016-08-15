#include "waveguide/cl/filter_structs.h"

const std::string cl_representation<biquad_memory>::value{
        R"(
#ifndef BIQUAD_MEMORY_DEFINITION__
#define BIQUAD_MEMORY_DEFINITION__
typedef struct {
    real array[)" +
        std::to_string(biquad_memory::order) + R"(];
} filter_memory_)" +
        std::to_string(biquad_memory::order) + R"(;

typedef filter_memory_)" +
        std::to_string(biquad_memory::order) + R"( biquad_memory;
#endif
)"};

const std::string cl_representation<biquad_coefficients>::value{
        R"(
#ifndef BIQUAD_COEFFICIENTS_DEFINITION__
#define BIQUAD_COEFFICIENTS_DEFINITION__
typedef struct {
    real b[)" +
        std::to_string(biquad_coefficients::order + 1) + R"(];
    real a[)" +
        std::to_string(biquad_coefficients::order + 1) + R"(];
} coefficients_)" +
        std::to_string(biquad_coefficients::order) + R"(;

typedef filter_coefficients_)" +
        std::to_string(biquad_coefficients::order) + R"( biquad_coefficients;
#endif
)"};

const std::string cl_representation<canonical_memory>::value{
        R"(
#ifndef CANONICAL_MEMORY_DEFINITION__
#define CANONICAL_MEMORY_DEFINITION__
typedef struct {
    real array[)" +
        std::to_string(canonical_memory::order) + R"(];
} filter_memory_)" +
        std::to_string(canonical_memory::order) + R"(;

typedef filter_memory_)" +
        std::to_string(canonical_memory::order) + R"( canonical_memory;
#endif
)"};

const std::string cl_representation<canonical_coefficients>::value{
        R"(
#ifndef CANONICAL_COEFFICIENTS_DEFINITION__
#define CANONICAL_COEFFICIENTS_DEFINITION__
typedef struct {
    real b[)" +
        std::to_string(canonical_coefficients::order + 1) + R"(];
    real a[)" +
        std::to_string(canonical_coefficients::order + 1) + R"(];
} coefficients_)" +
        std::to_string(canonical_coefficients::order) + R"(;

typedef filter_coefficients_)" +
        std::to_string(canonical_coefficients::order) +
        R"( canonical_coefficients;
#endif
)"};

