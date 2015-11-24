#include "cl_common.h"

#include <vector>

void print_device_info(const cl::Device& i) {
    Logger::log(i.getInfo<CL_DEVICE_NAME>());
    Logger::log("available: ", i.getInfo<CL_DEVICE_AVAILABLE>());
};

cl::Context get_context() {
    std::vector<cl::Platform> platform;
    cl::Platform::get(&platform);

    cl_context_properties cps[3] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)(platform[0])(), 0,
    };

    return cl::Context(CL_DEVICE_TYPE_GPU, cps);
}

cl::Device get_device(const cl::Context& context) {
    auto devices = context.getInfo<CL_CONTEXT_DEVICES>();

    Logger::log("## all devices:");

    for (auto& i : devices) {
        print_device_info(i);
    }

    auto device = devices.back();

    Logger::log("## used device:");
    print_device_info(device);

    return device;
}
