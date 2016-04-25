#pragma once

#include "common/cl_include.h"
#include "common/decibels.h"
#include "common/json_read_write.h"
#include "common/reduce.h"
#include "common/stl_wrappers.h"
#include "common/string_builder.h"

#include <cassert>
#include <cmath>

template <typename Archive>
void serialize(Archive& archive, cl_float3& m) {
    archive(cereal::make_nvp("x", m.s[0]),
            cereal::make_nvp("y", m.s[1]),
            cereal::make_nvp("z", m.s[2]));
}

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

    static constexpr BoundaryType port_index_to_boundary_type(unsigned int i) {
        return static_cast<BoundaryType>(1 << (i + 1));
    }

    typedef enum : cl_int {
        id_success = 0,
        id_inf_error = 1 << 0,
        id_nan_error = 1 << 1,
        id_outside_range_error = 1 << 2,
        id_outside_mesh_error = 1 << 3,
        id_suspicious_boundary_error = 1 << 4,
    } ErrorCode;

    static constexpr cl_uint NO_NEIGHBOR{~cl_uint{0}};

    struct __attribute__((aligned(8))) NodeStruct final {
        static constexpr int PORTS{6};
        cl_uint ports[PORTS]{};
        cl_float3 position{};
        cl_bool inside{};
        cl_int boundary_type{};
        cl_uint boundary_index{};

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(CEREAL_NVP(ports),
                    CEREAL_NVP(position),
                    CEREAL_NVP(position),
                    CEREAL_NVP(inside),
                    CEREAL_NVP(boundary_type),
                    CEREAL_NVP(boundary_index));
        }
    };

    struct __attribute__((aligned(8))) CondensedNodeStruct final {
        static constexpr int PORTS{6};
        cl_int boundary_type{};
        cl_uint boundary_index{};

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(CEREAL_NVP(boundary_type), CEREAL_NVP(boundary_index));
        }
    };

    using FilterReal = cl_double;

    constexpr static auto BIQUAD_ORDER = 2;

    template <int O>
    struct FilterMemory final {
        static constexpr int ORDER = O;
        FilterReal array[ORDER]{};

        bool operator==(const FilterMemory& rhs) const {
            return proc::equal(array, std::begin(rhs.array));
        }

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(CEREAL_NVP(array));
        }
    };

    using BiquadMemory = FilterMemory<BIQUAD_ORDER>;

    template <int O>
    struct FilterCoefficients final {
        static constexpr int ORDER = O;
        FilterReal b[ORDER + 1]{};
        FilterReal a[ORDER + 1]{};

        bool operator==(const FilterCoefficients& rhs) const {
            return proc::equal(b, std::begin(rhs.b)) &&
                   proc::equal(a, std::begin(rhs.a));
        }

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(CEREAL_NVP(b), CEREAL_NVP(a));
        }
    };

    using BiquadCoefficients = FilterCoefficients<BIQUAD_ORDER>;

    struct __attribute__((aligned(8))) BiquadMemoryArray final {
        static constexpr int BIQUAD_SECTIONS{3};
        BiquadMemory array[BIQUAD_SECTIONS]{};
    };

    struct __attribute__((aligned(8))) BiquadCoefficientsArray final {
        static constexpr int BIQUAD_SECTIONS =
            BiquadMemoryArray::BIQUAD_SECTIONS;
        BiquadCoefficients array[BIQUAD_SECTIONS]{};

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(CEREAL_NVP(array));
        }
    };

    using CanonicalMemory =
        FilterMemory<BiquadMemory::ORDER * BiquadMemoryArray::BIQUAD_SECTIONS>;
    using CanonicalCoefficients =
        FilterCoefficients<BiquadCoefficients::ORDER *
                           BiquadCoefficientsArray::BIQUAD_SECTIONS>;

    struct BoundaryData final {
        CanonicalMemory filter_memory{};
        cl_int coefficient_index{};

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(CEREAL_NVP(filter_memory), CEREAL_NVP(coefficient_index));
        }
    };

    template <int D>
    struct __attribute__((aligned(8))) BoundaryDataArray final {
        static constexpr int DIMENSIONS{D};
        BoundaryData array[DIMENSIONS]{};

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(CEREAL_NVP(array));
        }
    };

    using BoundaryDataArray1 = BoundaryDataArray<1>;
    using BoundaryDataArray2 = BoundaryDataArray<2>;
    using BoundaryDataArray3 = BoundaryDataArray<3>;

    static CanonicalCoefficients to_impedance_coefficients(
        const CanonicalCoefficients& c);

    static constexpr int PORTS = NodeStruct::PORTS;

    explicit RectangularProgram(const cl::Context& context,
                                bool build_immediate = false);

    auto get_kernel() const {
        int error;
        auto ret =
            cl::make_kernel<cl::Buffer,
                            cl::Buffer,
                            cl::Buffer,
                            cl_int3,
                            cl::Buffer,
                            cl::Buffer,
                            cl::Buffer,
                            cl::Buffer,
                            cl::Buffer,
                            cl::Buffer,
                            cl_float,
                            cl_float,
                            cl_float,
                            cl_ulong,
                            cl::Buffer,
                            cl::Buffer>(*this, "condensed_waveguide", &error);
        assert(error == CL_SUCCESS);
        return ret;
    }

    auto get_filter_test_kernel() const {
        return cl::make_kernel<cl::Buffer, cl::Buffer, cl::Buffer, cl::Buffer>(
            *this, "filter_test");
    }

    auto get_filter_test_2_kernel() const {
        return cl::make_kernel<cl::Buffer, cl::Buffer, cl::Buffer, cl::Buffer>(
            *this, "filter_test_2");
    }

    auto get_ghost_point_test_kernel() const {
        return cl::make_kernel<cl::Buffer, cl::Buffer, cl::Buffer>(
            *this, "ghost_point_test");
    }

    static CondensedNodeStruct condense(const NodeStruct& n);

    struct FilterDescriptor {
        double gain{0};
        double centre{0};
        double Q{0};

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(CEREAL_NVP(gain), CEREAL_NVP(centre), CEREAL_NVP(Q));
        }
    };

    using coefficient_generator =
        BiquadCoefficients (*)(const FilterDescriptor& n, double sr);

    static BiquadCoefficients get_peak_coefficients(const FilterDescriptor& n,
                                                    double sr) {
        auto A = decibels::db2a(n.gain / 2);
        auto w0 = 2.0 * M_PI * n.centre / sr;
        auto cw0 = cos(w0);
        auto sw0 = sin(w0);
        auto alpha = sw0 / 2.0 * n.Q;
        auto a0 = 1 + alpha / A;
        return RectangularProgram::BiquadCoefficients{
            {(1 + (alpha * A)) / a0, (-2 * cw0) / a0, (1 - alpha * A) / a0},
            {1, (-2 * cw0) / a0, (1 - alpha / A) / a0}};
    }

    template <size_t... Ix>
    constexpr static BiquadCoefficientsArray get_biquads_array(
        std::index_sequence<Ix...>,
        const std::array<FilterDescriptor,
                         BiquadCoefficientsArray::BIQUAD_SECTIONS>& n,
        double sr,
        coefficient_generator callback) {
        RectangularProgram::BiquadCoefficientsArray ret{
            {callback(std::get<Ix>(n), sr)...}};
        return ret;
    }
    constexpr static BiquadCoefficientsArray get_biquads_array(
        const std::array<FilterDescriptor,
                         BiquadCoefficientsArray::BIQUAD_SECTIONS>& n,
        double sr,
        coefficient_generator callback) {
        return get_biquads_array(
            std::make_index_sequence<
                BiquadCoefficientsArray::BIQUAD_SECTIONS>(),
            n,
            sr,
            callback);
    }

    constexpr static BiquadCoefficientsArray get_peak_biquads_array(
        const std::array<FilterDescriptor,
                         BiquadCoefficientsArray::BIQUAD_SECTIONS>& n,
        double sr) {
        return get_biquads_array(n, sr, get_peak_coefficients);
    }

    template <int A, int B>
    static FilterCoefficients<A + B> convolve(const FilterCoefficients<A>& a,
                                              const FilterCoefficients<B>& b) {
        auto ret = FilterCoefficients<A + B>{};
        for (auto i = 0; i != A + 1; ++i) {
            for (auto j = 0; j != B + 1; ++j) {
                ret.b[i + j] += a.b[i] * b.b[j];
                ret.a[i + j] += a.a[i] * b.a[j];
            }
        }
        return ret;
    }

    static CanonicalCoefficients convolve(const BiquadCoefficientsArray& a);

    template <size_t L>
    static constexpr bool is_stable(const std::array<double, L>& a) {
        auto rci = a[L - 1];
        if (std::abs(rci) >= 1) {
            return false;
        }

        constexpr auto next_size = L - 1;
        std::array<double, next_size> next_array;
        for (auto i = 0; i != next_size; ++i) {
            next_array[i] = (a[i] - rci * a[next_size - i]) / (1 - rci * rci);
        }
        return is_stable(next_array);
    }

    template <int L>
    static constexpr bool is_stable(const FilterCoefficients<L>& coeffs) {
        std::array<double, L + 1> denom;
        proc::copy(coeffs.a, denom.begin());
        return is_stable(denom);
    }

private:
    static constexpr int BIQUAD_SECTIONS = BiquadMemoryArray::BIQUAD_SECTIONS;
    static const std::string source;
};

JSON_OSTREAM_OVERLOAD(RectangularProgram::NodeStruct);

JSON_OSTREAM_OVERLOAD(RectangularProgram::CondensedNodeStruct);

template <int O>
JSON_OSTREAM_OVERLOAD(RectangularProgram::FilterCoefficients<O>);

JSON_OSTREAM_OVERLOAD(RectangularProgram::BiquadCoefficientsArray);

JSON_OSTREAM_OVERLOAD(RectangularProgram::BoundaryData);

template <int D>
JSON_OSTREAM_OVERLOAD(RectangularProgram::BoundaryDataArray<D>);

JSON_OSTREAM_OVERLOAD(RectangularProgram::FilterDescriptor);

//----------------------------------------------------------------------------//

template <>
constexpr bool RectangularProgram::is_stable(const std::array<double, 1>& a) {
    return true;
}
