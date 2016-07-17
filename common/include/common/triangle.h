#pragma once

#include "cl_include.h"

struct alignas(1 << 3) Triangle final {
    cl_ulong surface;
    cl_ulong v0;
    cl_ulong v1;
    cl_ulong v2;
};
