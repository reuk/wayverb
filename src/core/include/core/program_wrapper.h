#pragma once

#include "core/cl/common.h"

namespace core {

class program_wrapper final {
public:
    program_wrapper(const compute_context& cc, const std::string& source);
    program_wrapper(const compute_context& cc,
                    const std::pair<const char*, size_t>& source);
    program_wrapper(const compute_context& cc,
                    const std::vector<std::string>& sources);
    program_wrapper(const compute_context& cc,
                    const std::vector<std::pair<const char*, size_t>>& sources);

    program_wrapper(const program_wrapper&)                = default;
    program_wrapper& operator=(const program_wrapper&)     = default;

    program_wrapper(program_wrapper&&) noexcept            = delete;
    program_wrapper& operator=(program_wrapper&&) noexcept = delete;

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

}  // namespace core
