#pragma once

#include "cl_traits.h"

class compute_context final {
public:
    compute_context();

    cl::Context get_context() const;
    cl::Device get_device() const;

private:
    cl::Context context;
    cl::Device device;
};

template <typename T>
cl::Buffer load_to_buffer(const cl::Context& context, T t, bool read_only) {
    return cl::Buffer(context, std::begin(t), std::end(t), read_only);
}
