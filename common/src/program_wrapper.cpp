#include "common/program_wrapper.h"

#include <iostream>

program_wrapper::program_wrapper(const cl::Context& context,
                                 const cl::Device& device,
                                 const std::string& source)
        : program_wrapper(context,
                          device,
                          std::make_pair(source.data(), source.size())) {
}

program_wrapper::program_wrapper(const cl::Context& context,
                                 const cl::Device& device,
                                 const std::pair<const char*, size_t>& source)
        : program_wrapper(context,
                          device,
                          std::vector<std::pair<const char*, size_t>>{source}) {
}

program_wrapper::program_wrapper(const cl::Context& context,
                                 const cl::Device& device,
                                 const std::vector<std::string>& sources)
        : program_wrapper(context, device, [&sources] {
            std::vector<std::pair<const char*, size_t>> ret;
            ret.reserve(sources.size());
            for (const auto& source : sources) {
                ret.push_back(std::make_pair(source.data(), source.size()));
            }
            return ret;
        }()) {
}

program_wrapper::program_wrapper(
        const cl::Context& context,
        const cl::Device& device,
        const std::vector<std::pair<const char*, size_t>>& sources)
        : device(device)
        , program(context, sources) {
    build(device);
}

void program_wrapper::build(const cl::Device& device) const {
    try {
        program.build({device}, "-Werror");
    } catch (const cl::Error& e) {
        std::cerr << program.template getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)
                  << "\n"
                  << e.what() << std::endl;
        throw;
    }
}

cl::Device program_wrapper::get_device() const {
    return device;
}
