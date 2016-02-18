#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

class RectangularProgram : public cl::Program {
public:
    typedef enum : cl_int {
        id_none = 0,
        id_inside = 1 << 0,
        id_nx = 1 << 1,
        id_px = 1 << 2,
        id_ny = 1 << 3,
        id_py = 1 << 4,
        id_nz = 1 << 5,
        id_pz = 1 << 6,
        id_reentrant = 1 << 7,
    } BoundaryType;

    struct __attribute__((aligned(8))) NodeStruct final {
        static constexpr int PORTS{6};
        static constexpr cl_uint NO_NEIGHBOR{~cl_uint{0}};
        cl_uint ports[PORTS];
        cl_float3 position;
        cl_bool inside;
        cl_int bt;
        cl_uint boundary_index;
    };

    template <int O>
    struct FilterMemory final {
        static constexpr int ORDER = O;
        cl_float array[ORDER];
    };

    using BiquadMemory = FilterMemory<2>;

    template <int O>
    struct FilterCoefficients final {
        static constexpr int ORDER = O;
        cl_float b[ORDER + 1];
        cl_float a[ORDER + 1];
    };

    using BiquadCoefficients = FilterCoefficients<2>;

    struct __attribute__((aligned(8))) BiquadMemoryArray final {
        static constexpr int BIQUAD_SECTIONS{3};
        BiquadMemory array[BIQUAD_SECTIONS];
    };

    struct __attribute__((aligned(8))) BiquadCoefficientsArray final {
        static constexpr int BIQUAD_SECTIONS =
            BiquadMemoryArray::BIQUAD_SECTIONS;
        BiquadCoefficients array[BIQUAD_SECTIONS];
    };

    using CanonicalMemory =
        FilterMemory<BiquadMemory::ORDER * BiquadMemoryArray::BIQUAD_SECTIONS>;
    using CanonicalCoefficients =
        FilterCoefficients<BiquadCoefficients::ORDER *
                           BiquadCoefficientsArray::BIQUAD_SECTIONS>;

    struct BoundaryData final {
        CanonicalMemory filter_memory;
        cl_int coefficient_index;
    };

    template <int D>
    struct __attribute__((aligned(8))) BoundaryDataArray final {
        static constexpr int DIMENSIONS{D};
        BoundaryData array[DIMENSIONS];
    };

    using BoundaryDataArray1 = BoundaryDataArray<1>;
    using BoundaryDataArray2 = BoundaryDataArray<2>;
    using BoundaryDataArray3 = BoundaryDataArray<3>;

    static constexpr int PORTS = NodeStruct::PORTS;

    RectangularProgram(const cl::Context & context,
                       bool build_immediate = false);

    auto get_kernel() const {
        return cl::make_kernel<cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl_float,
                               cl_float,
                               cl_ulong,
                               cl::Buffer>(*this, "waveguide");
    }

    auto get_filter_test_kernel() const {
        return cl::make_kernel<cl::Buffer, cl::Buffer, cl::Buffer, cl::Buffer>(
            *this, "filter_test");
    }

    auto get_filter_test_2_kernel() const {
        return cl::make_kernel<cl::Buffer, cl::Buffer, cl::Buffer, cl::Buffer>(
            *this, "filter_test_2");
    }

private:
    static constexpr int BIQUAD_SECTIONS = BiquadMemoryArray::BIQUAD_SECTIONS;
    static const std::string source;
};
