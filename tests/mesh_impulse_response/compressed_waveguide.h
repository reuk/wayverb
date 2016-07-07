#pragma once

#include "common/cl_common.h"
#include "common/custom_program_base.h"

#include <array>

class compressed_rectangular_waveguide_program final
        : public custom_program_base {
public:
    explicit compressed_rectangular_waveguide_program(
            const cl::Context& context, const cl::Device& device);

    auto get_kernel() const {
        return custom_program_base::get_kernel<cl::Buffer, cl::Buffer>(
                "compressed_waveguide");
    }

private:
    static const std::string source;
};

class compressed_rectangular_waveguide final {
public:
    using kernel_type =
            decltype(std::declval<compressed_rectangular_waveguide_program>()
                             .get_kernel());
    compressed_rectangular_waveguide(
            const compressed_rectangular_waveguide_program& program,
            size_t dimension);
    std::vector<float> run_hard_source(std::vector<float>&& input);
    std::vector<float> run_soft_source(std::vector<float>&& input);

private:
    std::vector<float> run(
            std::vector<float>&& input,
            float (compressed_rectangular_waveguide::*step)(float));
    float run_hard_step(float i);
    float run_soft_step(float i);

    cl::CommandQueue queue;
    kernel_type kernel;
    const size_t dimension;

    std::array<cl::Buffer, 2> storage;
    cl::Buffer* current;
    cl::Buffer* previous;
};

