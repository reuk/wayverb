#include "common/custom_program_base.h"

#include <iostream>

custom_program_base::custom_program_base(const cl::Context& context,
                                         const std::string& source)
        : custom_program_base(context,
                              std::make_pair(source.data(), source.size())) {
}

custom_program_base::custom_program_base(
        const cl::Context& context,
        const std::pair<const char*, size_t>& source)
        : custom_program_base(
                  context,
                  std::vector<std::pair<const char*, size_t>>{source}) {
}

custom_program_base::custom_program_base(
        const cl::Context& context,
        const std::vector<std::pair<const char*, size_t>>& sources)
        : program(context, sources) {
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
