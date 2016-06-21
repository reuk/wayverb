#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

template <typename T>
class ThumbnailBuffer {
public:
    explicit ThumbnailBuffer(size_t size = 0, const T& t = T())
            : buffer(size, t)
            , fifo(size) {
    }
    virtual ~ThumbnailBuffer() noexcept = default;

    size_t size() const {
        return buffer.size();
    }

    size_t get_free_space() const {
        return fifo.getFreeSpace();
    }

    size_t get_num_ready() const {
        return fifo.getNumReady();
    }

    void clear() {
        fifo.reset();
    }

    template <typename It>
    void write(It begin, size_t num) {
        int begin_1, size_1, begin_2, size_2;
        fifo.prepareToWrite(num, begin_1, size_1, begin_2, size_2);
        assert(size_1 >= 0);
        assert(size_2 >= 0);
        std::copy(begin, begin + size_1, buffer.begin() + begin_1);
        std::copy(begin + size_1,
                  begin + size_1 + size_2,
                  buffer.begin() + begin_2);
        fifo.finishedWrite(size_1 + size_2);
    }

    template <typename It>
    void read(It begin, size_t num) {
        int begin_1, size_1, begin_2, size_2;
        fifo.prepareToRead(num, begin_1, size_1, begin_2, size_2);
        assert(size_1 >= 0);
        assert(size_2 >= 0);
        std::copy(buffer.begin() + begin_1,
                  buffer.begin() + begin_1 + size_1,
                  begin);
        std::copy(buffer.begin() + begin_2,
                  buffer.begin() + begin_2 + size_2,
                  begin + size_1);
        fifo.finishedRead(size_1 + size_2);
    }

private:
    std::vector<T> buffer;
    AbstractFifo fifo;
};

template <typename T>
class HopBuffer {
public:
    HopBuffer(size_t buffer_size, size_t hop_size, const T& t = T())
            : buffer(buffer_size, t)
            , hop_size(hop_size) {
        assert(hop_size <= buffer_size);
    }
    virtual ~HopBuffer() noexcept = default;

    size_t size() const {
        return buffer.size();
    }

    size_t get_hop_size() const {
        return hop_size;
    }

    template <typename In, typename Out>
    void step(In begin_in, Out begin_out) {
        write(begin_in);
        read(begin_out);
    }

private:
    template <typename It>
    void write(It begin) {
        auto num = hop_size;
        auto diff = std::min(buffer.size() - read_ptr, hop_size);
        std::copy(begin, begin + diff, buffer.begin() + read_ptr);
        std::copy(begin + diff, begin + num, buffer.begin());
        read_ptr = (read_ptr + hop_size) % buffer.size();
    }

    template <typename It>
    void read(It begin) {
        auto diff = buffer.size() - read_ptr;
        std::copy(buffer.begin() + read_ptr, buffer.end(), begin);
        std::copy(buffer.begin(), buffer.begin() + read_ptr, begin + diff);
    }

    std::vector<T> buffer;
    size_t hop_size;
    size_t read_ptr{0};
};

template <typename T>
class InputBufferedHopBuffer {
public:
    using value_type = T;

    InputBufferedHopBuffer(size_t buffer_size,
                           size_t window_size,
                           size_t hop_size,
                           const T& t = T())
            : input_buffer(buffer_size, t)
            , output_buffer(window_size, hop_size, t)
            , temporary_buffer(hop_size, t) {
    }
    virtual ~InputBufferedHopBuffer() noexcept = default;

    bool has_waiting_frames() const {
        return input_buffer.get_num_ready() >= output_buffer.get_hop_size();
    }

    size_t size() const {
        return input_buffer.size();
    }

    size_t get_window_size() const {
        return output_buffer.size();
    }

    size_t get_hop_size() const {
        return output_buffer.get_hop_size();
    }

    template <typename It>
    void write(It begin, size_t num) {
        input_buffer.write(begin, num);
    }

    template <typename It>
    void read(It begin, size_t num) {
        assert(has_waiting_frames());
        input_buffer.read(temporary_buffer.data(), temporary_buffer.size());
        output_buffer.step(temporary_buffer.data(), begin);
    }

private:
    ThumbnailBuffer<T> input_buffer;
    HopBuffer<T> output_buffer;
    std::vector<T> temporary_buffer;
};