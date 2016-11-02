#pragma once

#include "fftw3.h"

namespace frequency_domain {

class plan final {
public:
    plan(const fftwf_plan& p);
    ~plan() noexcept;

    operator const fftwf_plan&() const;

private:
    fftwf_plan p;
};

}  // namespace frequency_domain
