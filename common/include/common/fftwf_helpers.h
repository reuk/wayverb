#pragma once

#include "fftw3.h"

#include <algorithm>
#include <memory>

// In this episode:
// How to work with FFTW in (hopefully) not a dumb way in C++

struct FftwfPlan {
    FftwfPlan(const fftwf_plan& rhs);
    virtual ~FftwfPlan() noexcept;
    operator const fftwf_plan&() const;

private:
    fftwf_plan plan;
};

struct fftwf_ptr_destructor {
    template <typename T>
    void operator()(T t) const noexcept {
        fftwf_free(t);
    }
};

enum class fftwf_data_type { real, cplx };

template <fftwf_data_type T>
struct fftwf_type_trait;

template <>
struct fftwf_type_trait<fftwf_data_type::real> {
    using type = float;
    using ptr = std::unique_ptr<type[], fftwf_ptr_destructor>;
    using alloc_func_type = type*(size_t);
    static alloc_func_type& alloc;
};

template <>
struct fftwf_type_trait<fftwf_data_type::cplx> {
    using type = fftwf_complex;
    using ptr = std::unique_ptr<type[], fftwf_ptr_destructor>;
    using alloc_func_type = type*(size_t);
    static alloc_func_type& alloc;
};

template <fftwf_data_type T>
class fftwf_buffer final {
public:
    using type = typename fftwf_type_trait<T>::type;
    using ptr = typename fftwf_type_trait<T>::ptr;

    explicit fftwf_buffer(size_t buffer_size = 0)
            : buffer_size(buffer_size)
            , buffer(fftwf_type_trait<T>::alloc(buffer_size)) {
    }

    fftwf_buffer(const fftwf_buffer& rhs)
            : buffer_size(rhs.buffer_size)
            , buffer(fftwf_type_trait<T>::alloc(buffer_size)) {
        memcpy(data(), rhs.data(), buffer_size * sizeof(type));
    }
    fftwf_buffer& operator=(fftwf_buffer rhs) {
        swap(std::move(rhs));
        return *this;
    }

    fftwf_buffer(fftwf_buffer&& rhs) noexcept = default;
    fftwf_buffer& operator=(fftwf_buffer&& rhs) noexcept = default;

    void swap(fftwf_buffer&& rhs) noexcept {
        buffer_size = rhs.buffer_size;
        buffer = std::move(rhs.buffer);
    }

    size_t size() const noexcept {
        return buffer_size;
    }

    auto data() {
        return buffer.get();
    }

    const auto data() const {
        return buffer.get();
    }

    auto begin() {
        return buffer.get();
    }

    const auto begin() const {
        return buffer.get();
    }

    auto end() {
        return begin() + buffer_size;
    }

    const auto end() const {
        return begin() + buffer_size;
    }

    void zero() {
        memset(data(), 0, buffer_size * sizeof(type));
    }

private:
    size_t buffer_size;
    ptr buffer;
};

using fftwf_r = fftwf_buffer<fftwf_data_type::real>;
using fftwf_c = fftwf_buffer<fftwf_data_type::cplx>;