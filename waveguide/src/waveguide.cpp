#include "waveguide/waveguide.h"

#include "common/conversions.h"
#include "common/sinc.h"
#include "common/stl_wrappers.h"

#include "glog/logging.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>

//----------------------------------------------------------------------------//

namespace detail {

BufferTypeTrait<BufferType::cl>::storage_array_type
BufferTypeTrait<BufferType::cl>::create_waveguide_storage(
        const cl::Context& context, size_t nodes) {
    return {{cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * nodes),
             cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * nodes)}};
}

}  // namespace detail

//----------------------------------------------------------------------------//
