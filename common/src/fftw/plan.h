#pragma once

#include "fftw3.h"

namespace fftwf {

class plan final {
public:
    plan(const fftwf_plan& p);
    ~plan() noexcept;

    operator const fftwf_plan&() const;

private:
    fftwf_plan p;
};

}  // namespace fftwf
