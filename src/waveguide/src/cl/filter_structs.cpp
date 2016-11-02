#include "waveguide/cl/filter_structs.h"

const std::string core::cl_representation<memory_biquad>::value{
        R"(
typedef struct {
    filt_real array[)" +
        std::to_string(memory_biquad::order) + R"(];
} memory_)" +
        std::to_string(memory_biquad::order) + R"(;

typedef memory_)" +
        std::to_string(memory_biquad::order) + R"( memory_biquad;
)"};

const std::string core::cl_representation<coefficients_biquad>::value{
        R"(
typedef struct {
    filt_real b[)" +
        std::to_string(coefficients_biquad::order + 1) + R"(];
    filt_real a[)" +
        std::to_string(coefficients_biquad::order + 1) + R"(];
} coefficients_)" +
        std::to_string(coefficients_biquad::order) + R"(;

typedef coefficients_)" +
        std::to_string(coefficients_biquad::order) + R"( coefficients_biquad;
)"};

const std::string core::cl_representation<memory_canonical>::value{
        R"(
typedef struct {
    filt_real array[)" +
        std::to_string(memory_canonical::order) + R"(];
} memory_)" +
        std::to_string(memory_canonical::order) + R"(;

typedef memory_)" +
        std::to_string(memory_canonical::order) + R"( memory_canonical;
)"};

const std::string core::cl_representation<coefficients_canonical>::value{
        R"(
typedef struct {
    filt_real b[)" +
        std::to_string(coefficients_canonical::order + 1) + R"(];
    filt_real a[)" +
        std::to_string(coefficients_canonical::order + 1) + R"(];
} coefficients_)" +
        std::to_string(coefficients_canonical::order) + R"(;

typedef coefficients_)" +
        std::to_string(coefficients_canonical::order) +
        R"( coefficients_canonical;
)"};

