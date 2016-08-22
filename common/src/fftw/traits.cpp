#include "traits.h"

namespace fftwf {

type_trait<float>::alloc_func_type& type_trait<float>::alloc{fftwf_alloc_real};

type_trait<fftwf_complex>::alloc_func_type& type_trait<fftwf_complex>::alloc{
        fftwf_alloc_complex};

}  // namespace fftwf
