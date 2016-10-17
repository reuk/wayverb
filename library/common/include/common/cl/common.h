#pragma once

#include "common/cl/traits.h"

#include "utilities/aligned/vector.h"

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
aligned::vector<T> read_from_buffer(const cl::CommandQueue& queue,
                                    const cl::Buffer& buffer) {
    aligned::vector<T> ret(items_in_buffer<T>(buffer));
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

template <int N>
struct is_power_of_two final {
    using type = std::integral_constant<bool, N && !(N & (N - 1))>;
};

template <int N>
using is_power_of_two_t = typename is_power_of_two<N>::type;

template <int N>
constexpr auto is_power_of_two_v{is_power_of_two_t<N>{}};

template <typename T>
void fill_buffer(cl::CommandQueue& queue,
                 cl::Buffer& buffer,
                 const T& pattern) {
    const auto buffer_size{buffer.getInfo<CL_MEM_SIZE>()};
    constexpr auto item_size{sizeof(T)};
    static_assert(is_power_of_two_v<item_size>,
                  "item size must be a smallish power of two");
    if (buffer_size % item_size) {
        throw std::runtime_error{
                "fill_buffer: buffer size is not a multiple of pattern size"};
    }
    constexpr auto offset{0};
    queue.enqueueFillBuffer(buffer, pattern, offset, buffer_size);
}

template <typename T>
auto make_filled_buffer(cl::CommandQueue& queue, size_t size, const T& pattern) {
    cl::Buffer ret{queue.getInfo<CL_QUEUE_CONTEXT>(), CL_MEM_READ_WRITE, sizeof(T) * size};
    fill_buffer(queue, ret, pattern);
    return ret;
}
