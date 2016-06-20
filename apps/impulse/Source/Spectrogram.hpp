#pragma once

#include <memory>
#include <vector>

class BufferedSpectrogram {
public:
    BufferedSpectrogram(size_t buffer_size,
                        size_t window_size,
                        size_t hop_size);

    BufferedSpectrogram(BufferedSpectrogram&& rhs) noexcept;
    BufferedSpectrogram& operator=(BufferedSpectrogram&& rhs) noexcept;

    virtual ~BufferedSpectrogram() noexcept;

    bool has_waiting_frames() const;

    size_t get_window_size() const;
    size_t get_hop_size() const;

    void write(const float* begin, const float* end);
    std::vector<float> read_frame();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

//----------------------------------------------------------------------------//

class MultichannelBufferedSpectrogram {
public:
    MultichannelBufferedSpectrogram(size_t channels,
                                    size_t buffer_size,
                                    size_t window_size,
                                    size_t hop_size);
    virtual ~MultichannelBufferedSpectrogram() noexcept;

    bool has_waiting_frames() const;

    size_t get_num_channels() const;
    size_t get_window_size() const;
    size_t get_hop_size() const;

    void write(const float** begin, size_t num);
    void write(const float** begin, const float** end);
    std::vector<std::vector<float>> read_frame();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};