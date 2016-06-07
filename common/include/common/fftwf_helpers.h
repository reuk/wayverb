#pragma once

#include "fftw3.h"

#include <memory>

// In this episode:
// How to work with FFTW in not a dumb way in C++

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

using fftwf_r = std::unique_ptr<float[], fftwf_ptr_destructor>;
using fftwf_c = std::unique_ptr<fftwf_complex[], fftwf_ptr_destructor>;
