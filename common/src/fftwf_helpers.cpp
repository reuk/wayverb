#include "common/fftwf_helpers.h"

FftwfPlan::FftwfPlan(const fftwf_plan& plan)
        : plan(plan) {
}

FftwfPlan::~FftwfPlan() noexcept {
    fftwf_destroy_plan(plan);
}

FftwfPlan::operator const fftwf_plan&() const {
    return plan;
}
