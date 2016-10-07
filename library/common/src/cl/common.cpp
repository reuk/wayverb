#include "common/cl/common.h"

#include <iostream>

namespace {

cl::Context get_context(device_type type) {
    std::vector<cl::Platform> platform;
    cl::Platform::get(&platform);

    cl_context_properties cps[] = {
            CL_CONTEXT_PLATFORM,
            reinterpret_cast<cl_context_properties>((platform.front())()),
            0,
    };

    const auto convert_device_type{[](auto type) {
        switch (type) {
            case device_type::cpu: return CL_DEVICE_TYPE_CPU;
            case device_type::gpu: return CL_DEVICE_TYPE_GPU;
        }
    }};

    return cl::Context(convert_device_type(type), cps);
}

cl::Device get_device(const cl::Context& context) {
    auto devices{context.getInfo<CL_CONTEXT_DEVICES>()};

    devices.erase(
            std::remove_if(
                    begin(devices),
                    end(devices),
                    [](const cl::Device& i) {
                        return !i.getInfo<
                                CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>();
                    }),
            devices.end());

    if (devices.empty()) {
        throw std::runtime_error("no devices support double precision");
    }

    devices.erase(remove_if(begin(devices),
                            end(devices),
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

compute_context::compute_context() {
    //  I hate this. It is garbage.
    for (const auto& type : {device_type::gpu, device_type::cpu}) {
        try {
            *this = compute_context{type};
            return;
        } catch (...) {
        }
    }
    throw std::runtime_error{"no OpenCL context contains a usable device"};
}

compute_context::compute_context(device_type type)
        : compute_context(::get_context(type)) {}

compute_context::compute_context(const cl::Context& context)
        : compute_context(context, ::get_device(context)) {}

compute_context::compute_context(const cl::Context& context,
                                 const cl::Device& device)
        : context(context)
        , device(device) {}
