#pragma once

#include "common/cl_include.h"

template <int P>
class BasicDWMProgram final : public cl::Program {
public:
    static constexpr int PORTS{P};

    struct __attribute__((aligned(8))) NodeStruct final {
        static constexpr cl_uint NO_NEIGHBOR{~cl_uint{0}};
        cl_uint ports[PORTS];
        cl_float3 position;
        cl_bool inside;
    };

    explicit BasicDWMProgram(const cl::Context& context,
                             bool build_immediate = false)
            : Program(context, source, build_immediate) {
    }

    auto get_kernel() const {
        return cl::make_kernel<cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl_float,
                               cl_float,
                               cl_ulong,
                               cl::Buffer>(*this, "waveguide");
    };

private:
    static const std::string source;
};

template <int PORTS>
const std::string BasicDWMProgram<PORTS>::source{
#ifdef DIAGNOSTIC
    "#define DIAGNOSTIC\n"
#endif
    "#define PORTS (" +
    std::to_string(PORTS) +
    ")\n"
    R"(
#define NO_NEIGHBOR (~(uint)0)

typedef struct {
    uint ports[PORTS];
    float3 position;
    bool inside;
} Node;

kernel void waveguide
(   const global float * current
,   global float * previous
,   const global Node * nodes
,   const global float * transform_matrix
,   global float3 * velocity_buffer
,   float spatial_sampling_period
,   float T
,   ulong read
,   global float * output
) {
    size_t index = get_global_id(0);
    const global Node * node = nodes + index;

    if (! node->inside) {
        return;
    }

    float temp = 0;

    //  waveguide logic goes here
    for (int i = 0; i != PORTS; ++i) {
        uint port_index = node->ports[i];
        if (port_index != NO_NEIGHBOR && nodes[port_index].inside)
            temp += current[port_index];
    }

    temp /= (PORTS / 2);
    temp -= previous[index];

    previous[index] = temp;

    //  TODO move this stuff outside
    //  there's a small chance I'll do some out-of-order reading here I think

    if (index == read) {
        *output = previous[index];

        //  instantaneous intensity for mic modelling

        float differences[PORTS] = {0};
        for (int i = 0; i != PORTS; ++i) {
            uint port_index = node->ports[i];
            if (port_index != NO_NEIGHBOR && nodes[port_index].inside)
                differences[i] = (previous[port_index] - previous[index]) /
                    spatial_sampling_period;
        }

        //  the default for Eigen is column-major matrices
        //  so we'll assume that transform_matrix is column-major

        //  multiply differences by transformation matrix
        float3 multiplied = (float3)(0);
        for (int i = 0; i != PORTS; ++i) {
            multiplied += (float3)(
                transform_matrix[0 + i * 3],
                transform_matrix[1 + i * 3],
                transform_matrix[2 + i * 3]
            ) * differences[i];
        }

        //  muliply by -1/ambient_density
        float ambient_density = 1.225;
        multiplied /= -ambient_density;

        *velocity_buffer += T * multiplied;
    }
}
)"};

using TetrahedralProgram = BasicDWMProgram<4>;
