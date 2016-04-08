#pragma once

#include "cl_include.h"

struct __attribute__((aligned(8))) Triangle final {
    cl_ulong surface;
    cl_ulong v0;
    cl_ulong v1;
    cl_ulong v2;
};
