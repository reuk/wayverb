#include "plan.h"

namespace fftwf {

plan::plan(const fftwf_plan& p)
        : p(p) {}

plan::~plan() noexcept {
    fftwf_destroy_plan(p);
}

plan::operator const fftwf_plan&() const { return p; }

}  // namespace fftwf
