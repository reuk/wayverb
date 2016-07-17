#pragma once

#include "common/cl_common.h"

class custom_program_base {
public:
    custom_program_base(const cl::Context& context,
                        const cl::Device& device,
                        const std::string& source);
    custom_program_base(const cl::Context& context,
                        const cl::Device& device,
                        const std::pair<const char*, size_t>& source);
    custom_program_base(
            const cl::Context& context,
            const cl::Device& device,
            const std::vector<std::string>& sources);
    custom_program_base(
            const cl::Context& context,
            const cl::Device& device,
            const std::vector<std::pair<const char*, size_t>>& sources);

    custom_program_base(const custom_program_base&) = default;
    custom_program_base& operator=(const custom_program_base&) = default;
    custom_program_base(custom_program_base&&) noexcept = delete;
    custom_program_base& operator=(custom_program_base&&) noexcept = delete;
    virtual ~custom_program_base() noexcept = default;

    template <cl_program_info T>
    auto get_info() const {
        return program.getInfo<T>();
    }

    cl::Device get_device() const;

    template <typename... Ts>
    auto get_kernel(const char* kernel_name) const {
        int error;
        return cl::make_kernel<Ts...>(program, kernel_name, &error);
    }

private:
    void build(const cl::Device& device) const;

    cl::Device device;
    cl::Program program;
};
