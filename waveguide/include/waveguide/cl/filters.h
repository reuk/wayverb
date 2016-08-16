#pragma once

#include "waveguide/cl/filter_structs.h"

#include "common/cl/cat.h"

namespace cl_sources {
const std::string filters{
        std::string{} + R"(
#ifndef FILTERS_HEADER__
#define FILTERS_HEADER__
)" + cl_sources::cat +
        "\n#define BIQUAD_SECTIONS " + std::to_string(biquad_sections) +
        "\n#define BIQUAD_ORDER " + std::to_string(biquad_order) +
        "\n#define CANONICAL_FILTER_ORDER " +
        std::to_string(biquad_sections * 2) + "\n" + cl_representation_v<real> +
        cl_representation_v<memory_biquad> +
        cl_representation_v<coefficients_biquad> +
        cl_representation_v<memory_canonical> +
        cl_representation_v<coefficients_canonical> +
        cl_representation_v<biquad_memory_array> +
        cl_representation_v<biquad_coefficients_array> +
        R"(
#define FILTER_STEP(order)                                           \
    real CAT(filter_step_, order)(                                   \
            real input,                                              \
            global CAT(memory_, order) * m,                     \
            const global CAT(coefficients_, order) * c);        \
    real CAT(filter_step_, order)(                                   \
            real input,                                              \
            global CAT(memory_, order) * m,                     \
            const global CAT(coefficients_, order) * c) {       \
        const real output = (input * c->b[0] + m->array[0]) / c->a[0];     \
        for (int i = 0; i != order - 1; ++i) {                       \
            const real b = c->b[i + 1] == 0 ? 0 : c->b[i + 1] * input;     \
            const real a = c->a[i + 1] == 0 ? 0 : c->a[i + 1] * output;    \
            m->array[i]  = b - a + m->array[i + 1];                  \
        }                                                            \
        const real b = c->b[order] == 0 ? 0 : c->b[order] * input;         \
        const real a = c->a[order] == 0 ? 0 : c->a[order] * output;        \
        m->array[order - 1] = b - a;                                 \
        return output;                                               \
    }

FILTER_STEP(BIQUAD_ORDER);
FILTER_STEP(CANONICAL_FILTER_ORDER);

#define filter_step_biquad CAT(filter_step_, BIQUAD_ORDER)
#define filter_step_canonical CAT(filter_step_, CANONICAL_FILTER_ORDER)

float biquad_cascade(real input,
                     global biquad_memory_array* bm,
                     const global biquad_coefficients_array* bc);
float biquad_cascade(real input,
                     global biquad_memory_array* bm,
                     const global biquad_coefficients_array* bc) {
    for (int i = 0; i != BIQUAD_SECTIONS; ++i) {
        input = filter_step_biquad(input, bm->array + i, bc->array + i);
    }
    return input;
}

kernel void filter_test(
        const global float* input,
        global float* output,
        global biquad_memory_array* biquad_memory,
        const global biquad_coefficients_array* biquad_coefficients) {
    const size_t index  = get_global_id(0);
    output[index] = biquad_cascade(
            input[index], biquad_memory + index, biquad_coefficients + index);
}

kernel void filter_test_2(
        const global float* input,
        global float* output,
        global memory_canonical * canonical_memory,
        const global coefficients_canonical * canonical_coefficients) {
    const size_t index  = get_global_id(0);
    output[index] = filter_step_canonical(input[index],
                                          canonical_memory + index,
                                          canonical_coefficients + index);
}

#endif
)"};
}  // namespace cl_sources

