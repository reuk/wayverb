#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

typedef enum : cl_int {
    id_none = 0,
    id_nx = 1 << 1,
    id_px = 1 << 2,
    id_ny = 1 << 3,
    id_py = 1 << 4,
    id_nz = 1 << 5,
    id_pz = 1 << 6,
    id_reentrant = 1 << 7,
} BoundaryType;

template <int P>
struct __attribute__((aligned(8))) NodeStruct {
    static constexpr int PORTS{P};
    cl_int ports[PORTS];
    cl_float3 position;
    cl_bool inside;
    cl_int bt;
    cl_int boundary_index;
};

struct BiquadMemory {
    cl_float z1;
    cl_float z2;
};

struct BiquadCoefficients {
    cl_float b0;
    cl_float b1;
    cl_float b2;
    cl_float a1;
    cl_float a2;
};

struct __attribute__((aligned(8))) BiquadMemoryArray {
    static constexpr int BIQUAD_SECTIONS{3};
    BiquadMemory array[BIQUAD_SECTIONS];
};

struct __attribute__((aligned(8))) BiquadCoefficientsArray {
    static constexpr int BIQUAD_SECTIONS = BiquadMemoryArray::BIQUAD_SECTIONS;
    BiquadCoefficients array[BIQUAD_SECTIONS];
};

template <int D>
struct __attribute__((aligned(8))) BoundaryData {
    static constexpr int DIMENSIONS{D};
    cl_float sk_current[DIMENSIONS], sk_previous[DIMENSIONS];
    cl_float sm_current[DIMENSIONS], sm_previous[DIMENSIONS];
    cl_float ghost_current[DIMENSIONS], ghost_previous[DIMENSIONS];
    BiquadMemoryArray biquad_memory[DIMENSIONS];
};

using BoundaryData1 = BoundaryData<1>;
using BoundaryData2 = BoundaryData<2>;
using BoundaryData3 = BoundaryData<3>;
