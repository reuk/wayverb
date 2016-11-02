#include "waveguide/cl/filters.h"
#include "waveguide/cl/filter_structs.h"

namespace cl_sources {

const std::string filter_constants{
        "#define BIQUAD_SECTIONS " + std::to_string(biquad_sections) + "\n" +
        "#define BIQUAD_ORDER " + std::to_string(biquad_order) + "\n" +
        "#define CANONICAL_FILTER_ORDER " +
        std::to_string(biquad_sections * 2)};

const char* filters{R"(
#define CAT(a, b) PRIMITIVE_CAT(a, b)
#define PRIMITIVE_CAT(a, b) a##b

#define FILTER_STEP(order)                                                   \
    filt_real CAT(filter_step_, order)(                                      \
            filt_real input,                                                 \
            global CAT(memory_, order) * m,                                  \
            const global CAT(coefficients_, order) * c);                     \
    filt_real CAT(filter_step_, order)(                                      \
            filt_real input,                                                 \
            global CAT(memory_, order) * m,                                  \
            const global CAT(coefficients_, order) * c) {                    \
        const filt_real output = (input * c->b[0] + m->array[0]) / c->a[0];  \
        for (int i = 0; i != order - 1; ++i) {                               \
            const filt_real b = c->b[i + 1] == 0 ? 0 : c->b[i + 1] * input;  \
            const filt_real a = c->a[i + 1] == 0 ? 0 : c->a[i + 1] * output; \
            m->array[i] = b - a + m->array[i + 1];                           \
        }                                                                    \
        const filt_real b = c->b[order] == 0 ? 0 : c->b[order] * input;      \
        const filt_real a = c->a[order] == 0 ? 0 : c->a[order] * output;     \
        m->array[order - 1] = b - a;                                         \
        return output;                                                       \
    }

FILTER_STEP(BIQUAD_ORDER);
FILTER_STEP(CANONICAL_FILTER_ORDER);

#define filter_step_biquad CAT(filter_step_, BIQUAD_ORDER)
#define filter_step_canonical CAT(filter_step_, CANONICAL_FILTER_ORDER)

float biquad_cascade(filt_real input,
                     global biquad_memory_array* bm,
                     const global biquad_coefficients_array* bc);
float biquad_cascade(filt_real input,
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
    const size_t index = get_global_id(0);
    output[index] = biquad_cascade(
            input[index], biquad_memory + index, biquad_coefficients + index);
}

kernel void filter_test_2(
        const global float* input,
        global float* output,
        global memory_canonical* canonical_memory,
        const global coefficients_canonical* canonical_coefficients) {
    const size_t index = get_global_id(0);
    output[index] = filter_step_canonical(input[index],
                                          canonical_memory + index,
                                          canonical_coefficients + index);
}

)"};

}  // namespace cl_sources
