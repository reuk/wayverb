#pragma once

#include "core/cl/traits.h"

#include "utilities/aligned/vector.h"

namespace wayverb {
namespace core {

enum class device_type { cpu, gpu };

/// invariant: device is a valid device for the context
class compute_context final {
public:
    compute_context();
    explicit compute_context(device_type type);
    explicit compute_context(const cl::Context& context);
    compute_context(const cl::Context& context, const cl::Device& device);

    cl::Context context;
    cl::Device device;
};

template <typename T>
cl::Buffer load_to_buffer(const cl::Context& context, T t, bool read_only) {
    return cl::Buffer{context, std::begin(t), std::end(t), read_only};
}

template <typename T>
size_t items_in_buffer(const cl::Buffer& buffer) {
    return buffer.getInfo<CL_MEM_SIZE>() / sizeof(T);
}

template <typename T>
util::aligned::vector<T> read_from_buffer(const cl::CommandQueue& queue,
                                          const cl::Buffer& buffer) {
    util::aligned::vector<T> ret(items_in_buffer<T>(buffer));
    cl::copy(queue, buffer, ret.begin(), ret.end());
    return ret;
}

template <typename T>
T read_value(cl::CommandQueue& queue, const cl::Buffer& buffer, size_t index) {
    T ret;
    queue.enqueueReadBuffer(
            buffer, CL_TRUE, sizeof(T) * index, sizeof(T), &ret);
    return ret;
}

template <typename T>
void write_value(cl::CommandQueue& queue,
                 cl::Buffer& buffer,
                 size_t index,
                 T val) {
    queue.enqueueWriteBuffer(
            buffer, CL_TRUE, sizeof(T) * index, sizeof(T), &val);
}

}  // namespace core
}  // namespace wayverb
