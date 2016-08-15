#pragma once

#include "common/cl/cat.h"
#include "common/cl/cl_representation.h"
#include "common/cl_traits.h"
#include "common/stl_wrappers.h"

constexpr size_t biquad_order = 2;
constexpr size_t biquad_sections = 3;

//----------------------------------------------------------------------------//

using real = cl_double;

template <>
struct cl_representation<real> final {
    static constexpr const char* value{R"(
#ifndef REAL_DEFINITION__
#define REAL_DEFINITION__
typedef double real;
#endif
)"};
};

//----------------------------------------------------------------------------//

/// Just an array of reals to use as a delay line.
template <size_t o>
struct alignas(1 << 3) memory final {
    static constexpr size_t order = o;
    real array[order]{};
};

template <size_t D>
inline bool operator==(const memory<D>& a, const memory<D>& b) {
    return proc::equal(a.array, std::begin(b.array));
}

template <size_t D>
inline bool operator!=(const memory<D>& a, const memory<D>& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

/// IIR filter coefficient storage.
template <size_t o>
struct alignas(1 << 3) coefficients final {
    static constexpr size_t order = o;
    real b[order + 1]{};
    real a[order + 1]{};
};

template <size_t D>
inline bool operator==(const coefficients<D>& a, const coefficients<D>& b) {
    return proc::equal(a.a, std::begin(b.a)) &&
           proc::equal(a.b, std::begin(b.b));
}

template <size_t D>
inline bool operator!=(const coefficients<D>& a, const coefficients<D>& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

using biquad_memory = memory<biquad_order>;

template <>
struct cl_representation<biquad_memory> final {
    static const std::string value;
};

using biquad_coefficients = coefficients<biquad_order>;

template <>
struct cl_representation<biquad_coefficients> final {
    static const std::string value;
};

using canonical_memory = memory<biquad_memory::order * biquad_sections>;

template <>
struct cl_representation<canonical_memory> final {
    static const std::string value;
};

using canonical_coefficients =
        coefficients<biquad_coefficients::order * biquad_sections>;

template <>
struct cl_representation<canonical_coefficients> final {
    static const std::string value;
};

//----------------------------------------------------------------------------//

/// Several biquad delay lines in a row.
struct alignas(1 << 3) biquad_memory_array final {
    biquad_memory array[biquad_sections]{};
};

template <>
struct cl_representation<biquad_memory_array> final {
    static constexpr const char* value{R"(
#ifndef BIQUAD_MEMORY_ARRAY_DEFINITION__
#define BIQUAD_MEMORY_ARRAY_DEFINITION__
typedef struct {
    filter_memory_biquad array[BIQUAD_SECTIONS];
} biquad_memory_array;
#endif
)"};
};

//----------------------------------------------------------------------------//

/// Several sets of biquad parameters.
struct alignas(1 << 3) biquad_coefficients_array final {
    biquad_coefficients array[biquad_sections]{};
};

template <>
struct cl_representation<biquad_coefficients_array> final {
    static constexpr const char* value{R"(
#ifndef BIQUAD_COEFFICIENTS_ARRAY_DEFINITION__
#define BIQUAD_COEFFICIENTS_ARRAY_DEFINITION__
typedef struct {
    filter_coefficients_biquad array[BIQUAD_SECTIONS];
} biquad_coefficients_array;
#endif
)"};
};
