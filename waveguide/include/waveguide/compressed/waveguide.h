#pragma once

#include "common/aligned/vector.h"
#include "common/cl/common.h"
#include "common/program_wrapper.h"

#include <array>

class compressed_rectangular_waveguide_program final {
public:
    explicit compressed_rectangular_waveguide_program(
            const compute_context& cc);

    auto get_kernel() const {
        return program_wrapper_.get_kernel<cl::Buffer, cl::Buffer>(
                "compressed_waveguide");
    }

    template <cl_program_info T>
    auto get_info() const {
        return program_wrapper_.get_info<T>();
    }

    cl::Device get_device() const { return program_wrapper_.get_device(); }

private:
    program_wrapper program_wrapper_;
};

class compressed_rectangular_waveguide final {
public:
    using kernel_type =
            decltype(std::declval<compressed_rectangular_waveguide_program>()
                             .get_kernel());
    compressed_rectangular_waveguide(const compute_context& cc,
                                     size_t dimension);
    aligned::vector<float> run_hard_source(aligned::vector<float>&& input);
    aligned::vector<float> run_soft_source(aligned::vector<float>&& input);

private:
    aligned::vector<float> run(
            aligned::vector<float>&& input,
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
