#pragma once

#include <memory>
#include <vector>

class InputBufferedSpectrogram {
public:
    InputBufferedSpectrogram(size_t buffer_size,
                             size_t window_size,
                             size_t hop_size);
    virtual ~InputBufferedSpectrogram() noexcept;

    bool has_waiting_frames() const;

    size_t get_window_size() const;
    size_t get_hop_size() const;

    void write(const float* begin, const float* end);
    std::vector<float> read_frame();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};