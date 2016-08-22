#include "common/cl/common.h"
#include "common/stl_wrappers.h"
#include "common/string_builder.h"

#include <iostream>

namespace {

cl::Context get_context() {
    std::vector<cl::Platform> platform;
    cl::Platform::get(&platform);

    cl_context_properties cps[] = {
            CL_CONTEXT_PLATFORM,
            reinterpret_cast<cl_context_properties>((platform.front())()),
            0,
    };

    return cl::Context(CL_DEVICE_TYPE_GPU, cps);
    // return cl::Context(CL_DEVICE_TYPE_CPU, cps);
}

cl::Device get_device(const cl::Context& context) {
    auto devices{context.getInfo<CL_CONTEXT_DEVICES>()};

    devices.erase(
            proc::remove_if(
                    devices,
                    [](const cl::Device& i) {
                        return !i.getInfo<
                                CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>();
                    }),
            devices.end());

    if (devices.empty()) {
        throw std::runtime_error("no devices support double precision");
    }

    devices.erase(proc::remove_if(devices,
                                  [](const cl::Device& i) {
                                      return !i.getInfo<CL_DEVICE_AVAILABLE>();
                                  }),
                  devices.end());

    if (devices.empty()) {
        throw std::runtime_error("no suitable devices available");
    }

    const auto device{devices.front()};

    std::cerr << "device selected: " << device.getInfo<CL_DEVICE_NAME>()
              << '\n';

    return device;
}

}  // namespace

compute_context::compute_context()
        : context(::get_context())
        , device(::get_device(context)) {}

cl::Context compute_context::get_context() const { return context; }

cl::Device compute_context::get_device() const { return device; }
