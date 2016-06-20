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

fftwf_type_trait<fftwf_data_type::real>::alloc_func_type&
        fftwf_type_trait<fftwf_data_type::real>::alloc = fftwf_alloc_real;

fftwf_type_trait<fftwf_data_type::cplx>::alloc_func_type&
        fftwf_type_trait<fftwf_data_type::cplx>::alloc = fftwf_alloc_complex;