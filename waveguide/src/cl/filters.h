#pragma once

#include <string>

namespace cl_sources {

inline std::string get_filter_definitions(size_t BIQUAD_SECTIONS) {
    return std::string{"#define BIQUAD_SECTIONS " +
                       std::to_string(BIQUAD_SECTIONS) + "\n" +
                       "#define CANONICAL_FILTER_ORDER " +
                       std::to_string(BIQUAD_SECTIONS * 2) + "\n" +
                       R"(
#define CAT(a, b) PRIMITIVE_CAT(a, b)
#define PRIMITIVE_CAT(a, b) a##b

#define BIQUAD_ORDER 2

typedef double FilterReal;

#define TEMPLATE_FILTER_MEMORY(order) \
    typedef struct { FilterReal array[order]; } CAT(FilterMemory, order);

TEMPLATE_FILTER_MEMORY(BIQUAD_ORDER);
TEMPLATE_FILTER_MEMORY(CANONICAL_FILTER_ORDER);

#define TEMPLATE_FILTER_COEFFICIENTS(order) \
    typedef struct {                        \
        FilterReal b[order + 1];            \
        FilterReal a[order + 1];            \
    } CAT(FilterCoefficients, order);

TEMPLATE_FILTER_COEFFICIENTS(BIQUAD_ORDER);
TEMPLATE_FILTER_COEFFICIENTS(CANONICAL_FILTER_ORDER);

#define FilterMemoryBiquad CAT(FilterMemory, BIQUAD_ORDER)
#define FilterMemoryCanonical CAT(FilterMemory, CANONICAL_FILTER_ORDER)
#define FilterCoefficientsBiquad CAT(FilterCoefficients, BIQUAD_ORDER)
#define FilterCoefficientsCanonical \
    CAT(FilterCoefficients, CANONICAL_FILTER_ORDER)

typedef struct { FilterMemoryBiquad array[BIQUAD_SECTIONS]; } BiquadMemoryArray;
typedef struct {
    FilterCoefficientsBiquad array[BIQUAD_SECTIONS];
} BiquadCoefficientsArray;

#define FILTER_STEP(order)                                                 \
    FilterReal CAT(filter_step_, order)(                                   \
            FilterReal input,                                              \
            global CAT(FilterMemory, order) * m,                           \
            const global CAT(FilterCoefficients, order) * c);              \
    FilterReal CAT(filter_step_, order)(                                   \
            FilterReal input,                                              \
            global CAT(FilterMemory, order) * m,                           \
            const global CAT(FilterCoefficients, order) * c) {             \
        FilterReal output = (input * c->b[0] + m->array[0]) / c->a[0];     \
        for (int i = 0; i != order - 1; ++i) {                             \
            FilterReal b = c->b[i + 1] == 0 ? 0 : c->b[i + 1] * input;     \
            FilterReal a = c->a[i + 1] == 0 ? 0 : c->a[i + 1] * output;    \
            m->array[i]  = b - a + m->array[i + 1];                        \
        }                                                                  \
        FilterReal b        = c->b[order] == 0 ? 0 : c->b[order] * input;  \
        FilterReal a        = c->a[order] == 0 ? 0 : c->a[order] * output; \
        m->array[order - 1] = b - a;                                       \
        return output;                                                     \
    }

FILTER_STEP(BIQUAD_ORDER);
FILTER_STEP(CANONICAL_FILTER_ORDER);

#define filter_step_biquad CAT(filter_step_, BIQUAD_ORDER)
#define filter_step_canonical CAT(filter_step_, CANONICAL_FILTER_ORDER)

float biquad_cascade(FilterReal input,
                     global BiquadMemoryArray* bm,
                     const global BiquadCoefficientsArray* bc);
float biquad_cascade(FilterReal input,
                     global BiquadMemoryArray* bm,
                     const global BiquadCoefficientsArray* bc) {
    for (int i = 0; i != BIQUAD_SECTIONS; ++i) {
        input = filter_step_biquad(input, bm->array + i, bc->array + i);
    }
    return input;
}

kernel void filter_test(
        const global float* input,
        global float* output,
        global BiquadMemoryArray* biquad_memory,
        const global BiquadCoefficientsArray* biquad_coefficients) {
    size_t index  = get_global_id(0);
    output[index] = biquad_cascade(
            input[index], biquad_memory + index, biquad_coefficients + index);
}

kernel void filter_test_2(
        const global float* input,
        global float* output,
        global FilterMemoryCanonical* canonical_memory,
        const global FilterCoefficientsCanonical* canonical_coefficients) {
    size_t index  = get_global_id(0);
    output[index] = filter_step_canonical(input[index],
                                          canonical_memory + index,
                                          canonical_coefficients + index);
}
)"};
}
}  // namespace cl_sources
