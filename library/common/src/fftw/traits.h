#pragma once

#include "fftw3.h"

namespace fftwf {

template <typename T>
struct type_trait;

template <>
struct type_trait<float> final {
    using alloc_func_type = float*(size_t);
    static alloc_func_type& alloc;
};

template <>
struct type_trait<fftwf_complex> final {
    using alloc_func_type = fftwf_complex*(size_t);
    static alloc_func_type& alloc;
};

}  // namespace fftwf
