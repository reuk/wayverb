#include "common/custom_program_base.h"

#include <iostream>

custom_program_base::custom_program_base(const cl::Context& context,
                                         const cl::Device& device,
                                         const std::string& source)
        : custom_program_base(context,
                              device,
                              std::make_pair(source.data(), source.size())) {
}

custom_program_base::custom_program_base(
        const cl::Context& context,
        const cl::Device& device,
        const std::pair<const char*, size_t>& source)
        : custom_program_base(
                  context,
                  device,
                  std::vector<std::pair<const char*, size_t>>{source}) {
}

custom_program_base::custom_program_base(
        const cl::Context& context,
        const cl::Device& device,
        const std::vector<std::pair<const char*, size_t>>& sources)
        : device(device)
        , program(context, sources) {
    build(device);
}

void custom_program_base::build(const cl::Device& device) const {
    try {
        program.build({device}, "-Werror");
    } catch (const cl::Error& e) {
        std::cerr << program.template getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)
                  << "\n"
                  << e.what() << std::endl;
        throw;
    }
}

cl::Device custom_program_base::get_device() const {
    return device;
}
