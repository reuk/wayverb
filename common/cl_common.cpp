#include "cl_common.h"

#include <vector>

void print_device_info(const cl::Device& i) {
    LOG(INFO) << i.getInfo<CL_DEVICE_NAME>();
    LOG(INFO) << "available: " << i.getInfo<CL_DEVICE_AVAILABLE>();
};

cl::Context get_context() {
    std::vector<cl::Platform> platform;
    cl::Platform::get(&platform);

    cl_context_properties cps[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)(platform[0])(), 0,
    };

    return cl::Context(CL_DEVICE_TYPE_GPU, cps);
    // return cl::Context(CL_DEVICE_TYPE_CPU, cps);
}

cl::Device get_device(const cl::Context& context) {
    auto devices = context.getInfo<CL_CONTEXT_DEVICES>();

    LOG(INFO) << "## all devices:";

    for (auto& i : devices) {
        print_device_info(i);
    }

    auto device = devices.back();
    // auto device = devices.front();

    auto preferred_double_width =
        device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>();
    CHECK(preferred_double_width) << "device must support double precision";

    LOG(INFO) << "## used device:";
    print_device_info(device);

    return device;
}

ComputeContext::ComputeContext()
        : context(get_context())
        , device(get_device(context))
        , queue(context, device) {
}
